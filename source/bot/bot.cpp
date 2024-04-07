#include "bot/bot.hpp"

namespace kc {

size_t Bot::Bot::CountVoiceMembers(const dpp::guild& guild, dpp::snowflake channelId)
{
    size_t count = 0;
    for (const auto& voiceMember : guild.voice_members)
        count += (voiceMember.second.channel_id == channelId);
    return count;
}

bool Bot::Bot::UserInVoiceChannel(const dpp::guild& guild, dpp::snowflake channelId, dpp::snowflake userId)
{
    auto voiceMemberEntry = guild.voice_members.find(userId);
    if (voiceMemberEntry == guild.voice_members.end())
        return false;
    return (voiceMemberEntry->second.channel_id == channelId);
}

Bot::Bot::JoinStatus Bot::Bot::joinUserVoice(dpp::discord_client* client, const dpp::interaction& interaction, Info& info, const Youtube::Item& item)
{
    dpp::guild* guild = dpp::find_guild(interaction.guild_id);
    const dpp::user& user = interaction.get_issuing_user();
    auto userVoiceEntry = guild->voice_members.find(user.id);
    if (userVoiceEntry == guild->voice_members.end())
        return { JoinStatus::Result::CantJoin };

    dpp::voiceconn* botVoice = client->get_voice(guild->id);
    dpp::channel* userVoice = dpp::find_channel(userVoiceEntry->second.channel_id);
    if (botVoice)
    {
        if (botVoice->channel_id == userVoice->id)
            return { JoinStatus::Result::AlreadyJoined, userVoice };

        /*
        *   Bot and user are connected to different voice channels.
        *   Disconnect from current and connect to user's.
        */
        leaveVoice(client, interaction.get_guild());
        m_joinJobs.emplace(guild->id, JoinJob{ user, interaction.channel_id, item });
        return { JoinStatus::Result::Joining, userVoice };
    }

    guild->connect_member_voice(user.id, false, true);
    auto playerEntry = m_players.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(guild->id),
        std::forward_as_tuple(this, client, interaction, info)
    );
    if (item)
        playerEntry.first->second.addItem(item, user);
    return { JoinStatus::Result::Joined, userVoice };
}

dpp::message Bot::Bot::addItem(dpp::discord_client* client, const dpp::interaction& interaction, const std::string& itemId, const LogMessageFunction& logMessage, bool showRequester)
{
    return {};
}

Bot::Bot::Bot(std::shared_ptr<Config> config, bool registerCommands)
    : cluster(config->discordBotApiToken())
    , m_logger("bot", std::make_shared<spdlog::sinks::stdout_color_sink_mt>())
{
    on_log([this](const dpp::log_t& event)
    {
        static spdlog::logger logger("dpp", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        switch (event.severity)
        {
            case dpp::ll_warning:
                logger.warn(event.message);
                break;
            case dpp::ll_error:
                logger.error(event.message);
                break;
            case dpp::ll_critical:
                logger.critical(event.message);
                break;
        }
    });

    if (registerCommands)
    {
        on_ready([this](const dpp::ready_t& event)
        {
            if (dpp::run_once<struct RegisterCommands>())
            {
                using namespace Commands;
                std::vector<dpp::slashcommand> commands;

                /* /join */
                commands.push_back(dpp::slashcommand(English::Join::Name, English::Join::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Join::Name, Russian::Join::Description));

                /* /leave */
                commands.push_back(dpp::slashcommand(English::Leave::Name, English::Leave::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Leave::Name, Russian::Leave::Description));

                /* /session */
                commands.push_back(dpp::slashcommand(English::Session::Name, English::Session::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Session::Name, Russian::Session::Description));

                /* /play <what> */
                commands.push_back(dpp::slashcommand(English::Play::Name, English::Play::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Play::Name, Russian::Play::Description)
                    .add_option(dpp::command_option(dpp::co_string, English::Play::What::Name, English::Play::What::Description, true)
                        .add_localization(Russian::LocaleName, Russian::Play::What::Name, Russian::Play::What::Description)
                        .set_auto_complete(true)));

                /* /pause */
                commands.push_back(dpp::slashcommand(English::Pause::Name, English::Pause::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Pause::Name, Russian::Pause::Description));

                /* /resume */
                commands.push_back(dpp::slashcommand(English::Resume::Name, English::Resume::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Resume::Name, Russian::Resume::Description));

                /* /seek <timestamp-chapter> */
                commands.push_back(dpp::slashcommand(English::Seek::Name, English::Seek::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Seek::Name, Russian::Seek::Description)
                    .add_option(dpp::command_option(dpp::co_string, English::Seek::TimestampChapter::Name, English::Seek::TimestampChapter::Description, true)
                        .add_localization(Russian::LocaleName, Russian::Seek::TimestampChapter::Name, Russian::Seek::TimestampChapter::Description)
                        .set_auto_complete(true)));

                /* /shuffle */
                commands.push_back(dpp::slashcommand(English::Shuffle::Name, English::Shuffle::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Shuffle::Name, Russian::Shuffle::Description));

                /* /skip video, /skip playlist */
                commands.push_back(dpp::slashcommand(English::Skip::Name, "-", me.id)
                    .add_localization(Russian::LocaleName, Russian::Skip::Name, "-")
                    .add_option(dpp::command_option(dpp::co_sub_command, English::Skip::Video::Name, English::Skip::Video::Description)
                        .add_localization(Russian::LocaleName, Russian::Skip::Video::Name, Russian::Skip::Video::Description))
                    .add_option(dpp::command_option(dpp::co_sub_command, English::Skip::Playlist::Name, English::Skip::Playlist::Description)
                        .add_localization(Russian::LocaleName, Russian::Skip::Playlist::Name, Russian::Skip::Playlist::Description)));

                /* /clear */
                commands.push_back(dpp::slashcommand(English::Clear::Name, English::Clear::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Clear::Name, Russian::Clear::Description));

                /* /stop */
                commands.push_back(dpp::slashcommand(English::Stop::Name, English::Stop::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Stop::Name, Russian::Stop::Description));

                /* /settings */
                commands.push_back(dpp::slashcommand(English::Settings::Name, English::Settings::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Settings::Name, Russian::Settings::Description));

                /* /stats */
                commands.push_back(dpp::slashcommand(English::Stats::Name, English::Stats::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Stats::Name, Russian::Stats::Description));

                /* /set language, /set timeout */
                commands.push_back(dpp::slashcommand(English::Set::Name, "-", me.id)
                    .add_localization(Russian::LocaleName, Russian::Set::Name, "-")
                    .add_option(dpp::command_option(dpp::co_sub_command, English::Set::Language::Name, English::Set::Language::Description)
                        .add_localization(Russian::LocaleName, Russian::Set::Language::Name, Russian::Set::Language::Description)
                        .add_option(dpp::command_option(dpp::co_string, English::Set::Language::Language::Name, English::Set::Language::Language::Description, true)
                            .add_localization(Russian::LocaleName, Russian::Set::Language::Language::Name, Russian::Set::Language::Language::Description)
                            .add_choice(dpp::command_option_choice(English::Set::Language::Language::Label, std::string(English::Set::Language::Language::Id)))
                            .add_choice(dpp::command_option_choice(Russian::Set::Language::Language::Label, std::string(Russian::Set::Language::Language::Id)))))
                    .add_option(dpp::command_option(dpp::co_sub_command, English::Set::Timeout::Name, English::Set::Timeout::Description)
                        .add_localization(Russian::LocaleName, Russian::Set::Timeout::Name, Russian::Set::Timeout::Description)
                        .add_option(dpp::command_option(dpp::co_integer, English::Set::Timeout::Duration::Name, English::Set::Timeout::Duration::Description, true)
                            .add_localization(Russian::LocaleName, Russian::Set::Timeout::Duration::Name, Russian::Set::Timeout::Duration::Description))));

                m_logger.info("Registering {} slashcommand{}...", commands.size(), LocaleEn::Cardinal(commands.size()));
                global_bulk_command_create(commands);
            }
        });

        start(true);
        return;
    }

    on_ready([this](const dpp::ready_t& event)
    {
        if (dpp::run_once<struct ReadyMessage>())
        {
            m_logger.info("Ready");
            current_user_get_guilds([this](const dpp::confirmation_callback_t& event)
            {
                if (event.is_error())
                {
                    m_logger.error("Couldn't get bot guilds");
                    return;
                }

                const dpp::guild_map& guilds = std::get<dpp::guild_map>(event.value);
                switch (guilds.size())
                {
                    case 0:
                        m_logger.info("No guilds are served");
                        return;
                    case 1:
                        m_logger.info(
                            "Serving 1 guild: \"{}\" [{}]",
                            guilds.cbegin()->second.name,
                            static_cast<uint64_t>(guilds.cbegin()->second.id)
                        );
                        return;
                }

                m_logger.info("Serving {} guilds:", guilds.size());
                for (const auto& guild : guilds)
                    m_logger.info(" \"{}\" [{}]", guild.second.name, static_cast<uint64_t>(guild.second.id));
            });
        }
    });

    on_slashcommand([this](const dpp::slashcommand_t& event) {});

    on_autocomplete([this](const dpp::autocomplete_t& event) {});

    on_button_click([this](const dpp::button_click_t& event) {});

    on_select_click([this](const dpp::select_click_t& event) {});

    on_voice_ready([this](const dpp::voice_ready_t& event) {});

    on_voice_track_marker([this](const dpp::voice_track_marker_t& event) {});

    on_voice_state_update([this](const dpp::voice_state_update_t& event) {});
}

Bot::Bot::LeaveStatus Bot::Bot::leaveVoice(dpp::discord_client* client, const dpp::guild& guild, Locale::EndReason reason)
{
    dpp::voiceconn* botVoice = client->get_voice(guild.id);
    if (!botVoice)
        return { LeaveStatus::Result::CantLeave };

    const dpp::channel* disconnectedChannel = dpp::find_channel(botVoice->channel_id);
    if (reason != Locale::EndReason::None)
        m_players.find(guild.id)->second.endSession(reason);
    m_players.erase(guild.id);
    client->disconnect_voice(guild.id);
    return { LeaveStatus::Result::Left, disconnectedChannel };
}

} // namespace kc
