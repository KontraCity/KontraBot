#include "bot/bot.hpp"

// Library Boost.Regex
#include <boost/regex.hpp>

// Custom modules
#include "bot/locale/locale_en.hpp"
#include "bot/locale/locale_ru.hpp"
#include "bot/commands.hpp"
#include "common/config.hpp"
#include "common/utility.hpp"

namespace kc {

size_t Bot::Bot::CountVoiceMembers(const dpp::guild& guild, dpp::snowflake channelId)
{
    size_t count = 0;
    for (const auto& voiceMember : guild.voice_members)
        count += (voiceMember.second.channel_id == channelId);
    return count;
}

void Bot::Bot::presenceFunction()
{
    enum class PresenceType
    {
        GuildsServed,
        SessionsConducted,
        TracksPlayed,
        MaxPresenceTypes,
    };

    for (PresenceType type = PresenceType::GuildsServed; true; type = static_cast<PresenceType>(static_cast<int>(type) + 1))
    {
        if (type == PresenceType::MaxPresenceTypes)
            type = PresenceType::GuildsServed;

        switch (type)
        {
            case PresenceType::GuildsServed:
            {
                current_application_get([this](const dpp::confirmation_callback_t& event)
                {
                    if (event.is_error())
                    {
                        m_logger.error("Couldn't get current application for presence update");
                        return;
                    }

                    const dpp::application& application = std::get<dpp::application>(event.value);
                    set_presence(dpp::presence(dpp::ps_online, dpp::at_custom, fmt::format(
                        "{} guild{} served",
                        Utility::NiceString(application.approximate_guild_count),
                        LocaleEn::Cardinal(application.approximate_guild_count)
                    )));
                });
                break;
            }
            case PresenceType::SessionsConducted:
            {
                Stats globalStats = Info::GetGlobalStats();
                set_presence(dpp::presence(dpp::ps_online, dpp::at_custom, fmt::format(
                    "{} session{} conducted",
                    Utility::NiceString(globalStats.sessionsConducted),
                    LocaleEn::Cardinal(globalStats.sessionsConducted)
                )));
                break;
            }
            case PresenceType::TracksPlayed:
            {
                Stats globalStats = Info::GetGlobalStats();
                set_presence(dpp::presence(dpp::ps_online, dpp::at_custom, fmt::format(
                    "{} track{} played",
                    Utility::NiceString(globalStats.tracksPlayed),
                    LocaleEn::Cardinal(globalStats.tracksPlayed)
                )));
                break;
            }
        }

        pt::time_duration toNextMinute = Utility::TimeToNextMinute();
        if (toNextMinute.total_seconds() < 10)
            toNextMinute += pt::minutes(1);
        Utility::Sleep(toNextMinute.total_milliseconds() / 1000.0);
    }
}

void Bot::Bot::updateEphemeralToken(const dpp::confirmation_callback_t& confirmationEvent, std::string token)
{
    if (!confirmationEvent.is_error())
    {
        std::lock_guard lock(m_mutex);
        m_ephemeralTokens[confirmationEvent.get<dpp::message>().id] = token;
    }
}

Bot::Bot::PlayerEntry Bot::Bot::updatePlayerTextChannelId(dpp::snowflake guildId, dpp::snowflake channelId)
{
    PlayerEntry player = m_players.find(guildId);
    if (player != m_players.end())
        player->second.updateTextChannel(channelId);
    return player;
}

Bot::Info Bot::Bot::updateInfoProcessedInteractions(dpp::snowflake guildId)
{
    Info info(guildId);
    info.stats().interactionsProcessed += 1;
    return info;
}

bool Bot::Bot::playerControlsLocked(const dpp::guild& guild, dpp::snowflake userId)
{
    PlayerEntry playerEntry = m_players.find(guild.id);
    if (playerEntry == m_players.end())
    {
        // There is no playerEntry to lock when bot is not sitting in any voice channel
        return false;
    }

    auto voiceMemberEntry = guild.voice_members.find(userId);
    if (voiceMemberEntry == guild.voice_members.end())
    {
        // When bot is sitting in voice channel, playerEntry controls are locked for all users that are not sitting in any voice channel
        return true;
    }

    // Player controls are locked for all users that are not sitting in the same voice channel with bot
    return playerEntry->second.session().voiceChannelId != voiceMemberEntry->second.channel_id;
}

Bot::Bot::JoinStatus Bot::Bot::joinUserVoice(dpp::discord_client* client, const dpp::interaction& interaction, Info& info, const Youtube::Item& item)
{
    dpp::guild* guild = dpp::find_guild(interaction.guild_id);
    const dpp::user& user = interaction.get_issuing_user();
    auto userVoiceEntry = guild->voice_members.find(user.id);
    if (userVoiceEntry == guild->voice_members.end())
        return { JoinStatus::Result::UserNotInVoiceChannel };
    if (userVoiceEntry->second.channel_id == guild->afk_channel_id)
        return { JoinStatus::Result::UserInAfkChannel };

    dpp::voiceconn* botVoice = client->get_voice(guild->id);
    dpp::channel* userVoice = dpp::find_channel(userVoiceEntry->second.channel_id);

    if (botVoice)
    {
        if (botVoice->channel_id == userVoice->id)
            return { JoinStatus::Result::AlreadyJoined, userVoice };
        return { JoinStatus::Result::CantJoin, userVoice };
    }

    guild->connect_member_voice(user.id, false, true);
    auto emplacedPlayerEntry = m_players.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(guild->id),
        std::forward_as_tuple(this, client, interaction, userVoice->id, info)
    );
    if (item)
        emplacedPlayerEntry.first->second.addItem(item, user, info);
    return { JoinStatus::Result::Joined, userVoice };
}

dpp::message Bot::Bot::addItem(dpp::discord_client* client, const dpp::interaction& interaction, const std::string& itemId, const LogMessageFunction& logMessage, bool showRequester)
{
    const dpp::guild& guild = interaction.get_guild();
    const dpp::user& requester = interaction.get_issuing_user();

    try
    {
        if (boost::regex_search(itemId, boost::regex(Youtube::VideoConst::ValidateId)) ||
            boost::regex_search(itemId, boost::regex(Youtube::VideoConst::ExtractId)))
        {
            Youtube::Video video(itemId);
            std::lock_guard lock(m_mutex);
            Info info(guild.id);

            switch (video.type())
            {
                case Youtube::Video::Type::Normal:
                    break;
                case Youtube::Video::Type::Livestream:
                    m_logger.info(logMessage("Livestreams can't be played"));
                    return info.settings().locale->cantPlayLivestreams();
                case Youtube::Video::Type::Premiere:
                    m_logger.info(logMessage("Premieres can't be played"));
                    return info.settings().locale->cantPlayPremieres();
            }

            JoinStatus joinStatus = joinUserVoice(client, interaction, info, video);
            PlayerEntry playerEntry = m_players.find(guild.id);
            switch (joinStatus.result)
            {
                case JoinStatus::Result::AlreadyJoined:
                    playerEntry->second.addItem(video, requester, info);
                    break;
                case JoinStatus::Result::UserNotInVoiceChannel:
                    m_logger.info(logMessage("User not in voice channel"));
                    return info.settings().locale->userNotInVoiceChannel();
                case JoinStatus::Result::UserInAfkChannel:
                    m_logger.info(logMessage("User is sitting in an AFK channel"));
                    return info.settings().locale->cantPlayInAfkChannels();
                default:
                    break;
            }

            m_logger.info(logMessage(fmt::format("Added \"{}\" [{}]", video.title(), Utility::NiceString(video.duration()))));
            return info.settings().locale->itemAdded(video, playerEntry->second.paused(), showRequester ? interaction.get_issuing_user() : std::optional<dpp::user>{});
        }

        Youtube::Playlist playlist(itemId);
        std::lock_guard lock(m_mutex);
        Info info(guild.id);

        JoinStatus joinStatus = joinUserVoice(client, interaction, info, playlist);
        PlayerEntry playerEntry = m_players.find(guild.id);
        switch (joinStatus.result)
        {
            case JoinStatus::Result::AlreadyJoined:
                playerEntry->second.addItem(playlist, requester, info);
                break;
            case JoinStatus::Result::UserNotInVoiceChannel:
                m_logger.info(logMessage("User not in voice channel"));
                return info.settings().locale->userNotInVoiceChannel();
            case JoinStatus::Result::UserInAfkChannel:
                m_logger.info(logMessage("User is sitting in an AFK channel"));
                return info.settings().locale->cantPlayInAfkChannels();
            default:
                break;
        }

        m_logger.info(logMessage(fmt::format("Added \"{}\" [{} videos]", playlist.title(), Utility::NiceString(playlist.videoCount()))));
        return info.settings().locale->itemAdded(playlist, playerEntry->second.paused(), showRequester ? interaction.get_issuing_user() : std::optional<dpp::user>{});
    }
    catch (const Youtube::YoutubeError& error)
    {
        std::lock_guard lock(m_mutex);
        m_logger.error(logMessage(fmt::format("YouTube error: {}", error.what())));
        return Info(guild.id).settings().locale->youtubeError(error);
    }
    catch (const Youtube::LocalError& error)
    {
        std::lock_guard lock(m_mutex);
        m_logger.error(logMessage(fmt::format("Local error: {}", error.what())));
        return Info(guild.id).settings().locale->localError(error);
    }
    catch (const std::runtime_error& error)
    {
        std::lock_guard lock(m_mutex);
        m_logger.error(logMessage(fmt::format("Runtime error: {}", error.what())));
        return Info(guild.id).settings().locale->unknownError();
    }
    catch (const std::exception& error)
    {
        std::lock_guard lock(m_mutex);
        m_logger.error(logMessage(fmt::format("Unknown error: {}", error.what())));
        return Info(guild.id).settings().locale->unknownError();
    }
    catch (...)
    {
        std::lock_guard lock(m_mutex);
        m_logger.error(logMessage("Unknown error"));
        return Info(guild.id).settings().locale->unknownError();
    }
}

Bot::Bot::Bot(bool registerCommands)
    : cluster(Config::Instance->discordBotApiToken())
    , m_logger(Utility::CreateLogger("bot"))
{
    on_log(std::bind(&Bot::onLog, this, std::placeholders::_1, registerCommands));

    if (registerCommands)
    {
        on_ready([this](const dpp::ready_t& event)
        {
            if (dpp::run_once<struct RegisterCommands>())
            {
                std::vector<dpp::slashcommand> commands = Commands::GetCommands(me.id);
                m_logger.info("Registering {} slashcommand{}...", commands.size(), LocaleEn::Cardinal(commands.size()));
                global_bulk_command_create(commands);
            }
        });

        start(true);
        return;
    }

    on_autocomplete(std::bind(&Bot::onAutocomplete, this, std::placeholders::_1));

    on_button_click(std::bind(&Bot::onButtonClick, this, std::placeholders::_1));

    on_ready(std::bind(&Bot::onReady, this, std::placeholders::_1));

    on_message_create(std::bind(&Bot::onMessageCreate, this, std::placeholders::_1));

    on_select_click(std::bind(&Bot::onSelectClick, this, std::placeholders::_1));

    on_slashcommand(std::bind(&Bot::onSlashcommand, this, std::placeholders::_1));

    on_voice_ready(std::bind(&Bot::onVoiceReady, this, std::placeholders::_1));

    on_voice_server_update(std::bind(&Bot::onVoiceServerUpdate, this, std::placeholders::_1));

    on_voice_state_update(std::bind(&Bot::onVoiceStateUpdate, this, std::placeholders::_1));

    on_voice_track_marker(std::bind(&Bot::onVoiceTrackMarker, this, std::placeholders::_1));
}

Bot::Bot::LeaveStatus Bot::Bot::leaveVoice(dpp::discord_client* client, const dpp::guild& guild, Info& info, Locale::EndReason reason)
{
    dpp::voiceconn* botVoice = client->get_voice(guild.id);
    if (!botVoice)
        return { LeaveStatus::Result::BotNotInVoiceChannel };

    const dpp::channel* disconnectedChannel = dpp::find_channel(botVoice->channel_id);
    m_players.find(guild.id)->second.endSession(info, reason);
    m_players.erase(guild.id);
    return { LeaveStatus::Result::Left, disconnectedChannel };
}

} // namespace kc
