#include "bot/bot.hpp"

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
    enum class PresenceType {
        GuildsCount,
        SessionsConducted,
        TracksPlayed,
        MaxPresenceTypes,
    };

    for (PresenceType type = PresenceType::GuildsCount; true; type = static_cast<PresenceType>((int)type + 1))
    {
        if (type == PresenceType::MaxPresenceTypes)
            type = PresenceType::GuildsCount;

        switch (type)
        {
            case PresenceType::GuildsCount:
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

Bot::Bot::PlayerEntry Bot::Bot::updatePlayerTextChannelId(dpp::snowflake guildId, dpp::snowflake channelId)
{
    PlayerEntry player = m_players.find(guildId);
    if (player != m_players.end())
        player->second.updateTextChannel(channelId);
    return player;
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

dpp::message Bot::Bot::addItem(dpp::discord_client* client, const dpp::interaction& interaction, const std::string& itemId, const LogMessageFunction& logMessage, Info& info, bool showRequester)
{
    const dpp::guild& guild = interaction.get_guild();
    const dpp::user& requester = interaction.get_issuing_user();

    try
    {
        if (boost::regex_search(itemId, boost::regex(Youtube::VideoConst::ValidateId)) ||
            boost::regex_search(itemId, boost::regex(Youtube::VideoConst::ExtractId)))
        {
            Youtube::Video video(itemId);
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
        m_logger.error(logMessage(fmt::format("YouTube error: {}", error.what())));
        return info.settings().locale->youtubeError(error);
    }
    catch (const Youtube::LocalError& error)
    {
        m_logger.error(logMessage(fmt::format("Local error: {}", error.what())));
        return info.settings().locale->localError(error);
    }
    catch (const std::runtime_error& error)
    {
        m_logger.error(logMessage(fmt::format("Runtime error: {}", error.what())));
        return info.settings().locale->unknownError();
    }
    catch (const std::exception& error)
    {
        m_logger.error(logMessage(fmt::format("Unknown error: {}", error.what())));
        return info.settings().locale->unknownError();
    }
    catch (...)
    {
        m_logger.error(logMessage("Unknown error"));
        return info.settings().locale->unknownError();
    }
}

Bot::Bot::Bot(bool registerCommands)
    : cluster(Config::Instance->discordBotApiToken())
    , m_logger(Utility::CreateLogger("bot"))
{
    using namespace CommandsConst;

    on_log([this, registerCommands](const dpp::log_t& event)
    {
        static spdlog::logger logger = Utility::CreateLogger("dpp");
        switch (event.severity)
        {
            case dpp::ll_warning:
            {
                /* Filtering needless warnings */
                if (event.message.find("You have attached an event to cluster::on_message_create()") != std::string::npos)
                    break;
                if (registerCommands && event.message.find("Shard terminating due to cluster shutdown") != std::string::npos)
                    break;
                if (event.message.find("Remote site requested reconnection") != std::string::npos)
                    break;
                if (event.message.find("Terminating voice connection") != std::string::npos)
                    break;
                if (event.message.find("Success") != std::string::npos)
                    break;

                logger.warn(event.message);
                break;
            }
            case dpp::ll_error:
            {
                logger.error(event.message);
                break;
            }
            case dpp::ll_critical:
            {
                logger.critical(event.message);
                break;
            }
            default:
            {
                break;
            }
        }
    });

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

    on_ready([this](const dpp::ready_t& event)
    {
        // Start presence thread
        m_presenceThread = std::thread(&Bot::presenceFunction, this);

        if (dpp::run_once<struct ReadyMessage>())
        {
            global_commands_get([this](const dpp::confirmation_callback_t& event)
            {
                if (event.is_error())
                {
                    m_logger.critical("Couldn't get bot commands");
                    return;
                }

                try
                {
                    const dpp::slashcommand_map& commands = std::get<dpp::slashcommand_map>(event.value);
                    Commands::Instance->parse(commands);
                }
                catch (const std::runtime_error& error)
                {
                    m_logger.critical("Couldn't parse bot commands: {}", error.what());
                    return;
                }

                m_logger.info("Ready: logged in as {}", me.format_username());
                current_user_get_guilds([this](const dpp::confirmation_callback_t& event)
                {
                    if (event.is_error())
                    {
                        m_logger.error("Couldn't get bot guilds");
                        return;
                    }

                    const dpp::guild_map& guilds = std::get<dpp::guild_map>(event.value);
                    if (guilds.empty())
                    {
                        m_logger.info("No guilds are served");
                        return;
                    }
                    else if (guilds.size() == 1)
                    {
                        m_logger.info(
                            "Serving 1 guild: \"{}\" [{}]",
                            guilds.cbegin()->second.name,
                            static_cast<uint64_t>(guilds.cbegin()->second.id)
                        );
                        return;
                    }

                    m_logger.info("Serving {} guilds:", guilds.size());
                    for (const auto& guild : guilds)
                        m_logger.info("    \"{}\" [{}]", guild.second.name, static_cast<uint64_t>(guild.second.id));
                });
            });
        }
    });

    on_message_create([this](const dpp::message_create_t& event)
    {
        if (event.msg.content.empty() || event.msg.content.find(fmt::format("<@{}>", me.id)) == std::string::npos)
            return;

        dpp::guild* guild = dpp::find_guild(event.msg.guild_id);
        event.reply(Info(guild->id).settings().locale->mention());
        m_logger.info(
            "\"{}\" / \"{}\": Message: \"{}\", replying to mention",
            guild->name,
            event.msg.author.format_username(),
            event.msg.content
        );
    });

    on_slashcommand([this](const dpp::slashcommand_t& event)
    {
        dpp::command_interaction interaction = event.command.get_command_interaction();
        const dpp::guild& guild = event.command.get_guild();
        PlayerEntry playerEntry = updatePlayerTextChannelId(guild.id, event.command.channel_id);
        Info info(guild.id);
        ++info.stats().interactionsProcessed;

        const LogMessageFunction logMessage = [event, interaction, guild](const std::string& message)
        {
            if (!interaction.options.empty())
            {
                if (interaction.name == Play::Name || interaction.name == Seek::Name)
                {
                    return fmt::format(
                        "\"{}\" / \"{}\": /{} \"{}\": {}",
                        guild.name,
                        event.command.usr.format_username(),
                        interaction.name,
                        std::get<std::string>(interaction.options[0].value),
                        message
                    );
                }

                if (interaction.name == Skip::Name)
                {
                    return fmt::format(
                        "\"{}\" / \"{}\": /{} {}: {}",
                        guild.name,
                        event.command.usr.format_username(),
                        interaction.name,
                        interaction.options[0].name,
                        message
                    );
                }

                if (interaction.name == Set::Name)
                {
                    if (interaction.options[0].options[0].name == Set::Language::Language::Name ||
                        interaction.options[0].options[0].name == Set::ChangeStatus::Change::Name)
                    {
                        return fmt::format(
                            "\"{}\" / \"{}\": /{} {} \"{}\": {}",
                            guild.name,
                            event.command.usr.format_username(),
                            interaction.name,
                            interaction.options[0].name,
                            std::get<std::string>(interaction.options[0].options[0].value),
                            message
                        );
                    }

                    if (interaction.options[0].options[0].name == Set::Timeout::Duration::Name)
                    {
                        return fmt::format(
                            "\"{}\" / \"{}\": /{} {} {}: {}",
                            guild.name,
                            event.command.usr.format_username(),
                            interaction.name,
                            interaction.options[0].name,
                            std::get<int64_t>(interaction.options[0].options[0].value),
                            message
                        );
                    }
                }
            }

            return fmt::format(
                "\"{}\" / \"{}\": /{}: {}",
                guild.name,
                event.command.usr.format_username(),
                interaction.name,
                message
            );
        };

        if (interaction.name == CommandsConst::Help::Name)
        {
            event.reply(info.settings().locale->help());
            m_logger.info(logMessage("Showing help"));
            return;
        }

        if (interaction.name == CommandsConst::Session::Name)
        {
            if (playerEntry == m_players.end())
            {
                event.reply(info.settings().locale->botNotInVoiceChannel());
                m_logger.info(logMessage("Not in voice channel"));
                return;
            }

            event.reply(info.settings().locale->session(playerEntry->second.session()));
            m_logger.info(logMessage("Showing player's session"));
            return;
        }

        if (interaction.name == CommandsConst::Settings::Name)
        {
            event.reply(info.settings().locale->settings(info.settings()));
            m_logger.info(logMessage("Showing guild's settings"));
            return;
        }

        if (interaction.name == CommandsConst::Stats::Name)
        {
            event.reply(info.settings().locale->stats(info.stats()));
            m_logger.info(logMessage("Showing guild's stats"));
            return;
        }

        if (interaction.name == Set::Name)
        {
            const std::string& subcommand = interaction.options[0].name;
            if (subcommand == Set::Language::Name)
            {
                info.settings().locale = Locale::Create(std::get<std::string>(interaction.options[0].options[0].value));
                if (playerEntry != m_players.end())
                    playerEntry->second.updateVoiceStatus(info);
                event.reply(info.settings().locale->soBeIt());
                m_logger.info(logMessage(fmt::format("Locale set to \"{}\"", info.settings().locale->name())));
                return;
            }

            if (subcommand == Set::Timeout::Name)
            {
                int64_t timeoutDuration = std::get<int64_t>(interaction.options[0].options[0].value);
                if (timeoutDuration < 1)
                {
                    event.reply(info.settings().locale->badTimeoutDuration());
                    m_logger.info(logMessage("Bad timeout duration"));
                    return;
                }

                if (timeoutDuration > 120)
                {
                    event.reply(info.settings().locale->longTimeoutDuration());
                    m_logger.info(logMessage("Timeout duration is too long"));
                    return;
                }

                info.settings().timeoutMinutes = timeoutDuration;
                if (playerEntry != m_players.end())
                    playerEntry->second.updateTimeout(info);
                event.reply(info.settings().locale->timeoutDurationSet(timeoutDuration));
                m_logger.info(logMessage(fmt::format("Timeout duration is set to {}", Utility::NiceString(pt::time_duration(0, timeoutDuration, 0)))));
                return;
            }

            if (subcommand == Set::ChangeStatus::Name)
            {
                info.settings().changeStatus = std::get<std::string>(interaction.options[0].options[0].value) == "yes";
                if (playerEntry != m_players.end())
                    playerEntry->second.updateVoiceStatus(info);
                event.reply(info.settings().locale->soBeIt());
                m_logger.info(logMessage(fmt::format("Change status set to [{}]", info.settings().changeStatus ? "yes" : "no")));
                return;
            }

            event.reply(info.settings().locale->unknownCommand());
            m_logger.error(logMessage(fmt::format("Unknown subcommand: \"{}\"", subcommand)));
            return;
        }

        if (interaction.name == Join::Name)
        {
            JoinStatus joinStatus = joinUserVoice(event.from, event.command, info);
            switch (joinStatus.result)
            {
                case JoinStatus::Result::Joined:
                    event.reply(info.settings().locale->joining(joinStatus.channel->id));
                    m_logger.info(logMessage(fmt::format("Joining #{}", joinStatus.channel->name)));
                    return;
                case JoinStatus::Result::AlreadyJoined:
                    event.reply(info.settings().locale->alreadyJoined(joinStatus.channel->id));
                    m_logger.info(logMessage(fmt::format("Already joined #{}", joinStatus.channel->name)));
                    return;
                case JoinStatus::Result::CantJoin:
                    event.reply(info.settings().locale->cantJoin());
                    m_logger.info(logMessage("Can't join"));
                    return;
                case JoinStatus::Result::UserNotInVoiceChannel:
                    event.reply(info.settings().locale->userNotInVoiceChannel());
                    m_logger.info(logMessage("User not in voice channel"));
                    return;
                case JoinStatus::Result::UserInAfkChannel:
                    event.reply(info.settings().locale->cantPlayInAfkChannels());
                    m_logger.info(logMessage("User is sitting in AFK channel"));
                    return;
            }
        }

        if (playerControlsLocked(guild, event.command.usr.id))
        {
            event.reply(info.settings().locale->onlyUsersWithMeCanControlPlayer());
            m_logger.info(logMessage("Player controls are locked for user"));
            return;
        }

        if (interaction.name == Leave::Name)
        {
            std::optional<kc::Bot::Session> session;
            if (playerEntry != m_players.end())
                session.emplace(playerEntry->second.session());

            LeaveStatus leaveStatus = leaveVoice(event.from, guild, info, Locale::EndReason::UserRequested);
            switch (leaveStatus.result)
            {
                case LeaveStatus::Result::Left:
                {
                    dpp::message message = info.settings().locale->left(leaveStatus.channel->id);
                    message.add_embed(info.settings().locale->sessionEnd(info.settings(), Locale::EndReason::UserRequested, *session).embeds[0]);
                    event.reply(message);
                    m_logger.info(logMessage(fmt::format("Left #{}", leaveStatus.channel->name)));
                    return;
                }
                case LeaveStatus::Result::BotNotInVoiceChannel:
                {
                    event.reply(info.settings().locale->botNotInVoiceChannel());
                    m_logger.info(logMessage("Not in voice channel"));
                    return;
                }
            }
        }

        if (interaction.name == Play::Name)
        {
            const std::string& whatOption = std::get<std::string>(interaction.options[0].value);
            boost::smatch matches, videoMatches, playlistMatches;
            if (boost::regex_search(whatOption, videoMatches, boost::regex(Youtube::VideoConst::ExtractId)) &&
                boost::regex_search(whatOption, playlistMatches, boost::regex(Youtube::PlaylistConst::ExtractId)))
            {
                event.reply(info.settings().locale->ambiguousPlay(videoMatches.str(1), playlistMatches.str(1)));
                event.get_original_response([this, event](const dpp::confirmation_callback_t& confirmationEvent)
                {
                    if (!confirmationEvent.is_error())
                        m_ephemeralTokens[confirmationEvent.get<dpp::message>().id] = event.command.token;
                });
                m_logger.info(logMessage("Ambiguous"));
                return;
            }

            if (boost::regex_search(whatOption, boost::regex(Youtube::VideoConst::ExtractId)) ||
                boost::regex_search(whatOption, boost::regex(Youtube::PlaylistConst::ExtractId)))
            {
                event.reply(addItem(event.from, event.command, whatOption, logMessage, info));
                return;
            }

            event.thinking(true);
            Youtube::Results results = Youtube::Search(whatOption);
            event.edit_original_response(info.settings().locale->search(results), [this, event](const dpp::confirmation_callback_t& confirmationEvent)
            {
                if (!confirmationEvent.is_error())
                    m_ephemeralTokens[confirmationEvent.get<dpp::message>().id] = event.command.token;
            });
            m_logger.info(logMessage(fmt::format("{} result{}", results.size(), LocaleEn::Cardinal(results.size()))));
            return;
        }

        if (interaction.name == Pause::Name)
        {
            if (playerEntry == m_players.end())
            {
                event.reply(info.settings().locale->botNotInVoiceChannel());
                m_logger.info(logMessage("Not in voice channel"));
                return;
            }

            kc::Bot::Session session = playerEntry->second.session();
            if (!session.playingVideo)
            {
                event.reply(info.settings().locale->nothingIsPlaying());
                m_logger.info(logMessage("Nothing is playing"));
                return;
            }

            bool paused = playerEntry->second.pauseResume(info);
            event.reply(info.settings().locale->paused(session.playingVideo->video, paused));
            m_logger.info(logMessage(fmt::format(
                "{} \"{}\" [{}]",
                paused ? "Paused" : "Resumed",
                session.playingVideo->video.title(),
                Utility::NiceString(session.playingVideo->video.duration())
            )));
            return;
        }

        if (interaction.name == Seek::Name)
        {
            if (playerEntry == m_players.end())
            {
                event.reply(info.settings().locale->botNotInVoiceChannel());
                m_logger.info(logMessage("Not in voice channel"));
                return;
            }

            kc::Bot::Session session = playerEntry->second.session();
            if (!session.playingVideo)
            {
                event.reply(info.settings().locale->nothingIsPlaying());
                m_logger.info(logMessage("Nothing is playing"));
                return;
            }

            const std::string& timestampChapterOption = std::get<std::string>(interaction.options[0].value);
            boost::smatch matches;
            if (boost::regex_match(timestampChapterOption, matches, boost::regex(R"((?:(?:(\d+?):)?(\d+?):)?(\d+))")))
            {
                try
                {
                    std::string hours = matches.str(1);
                    std::string minutes = matches.str(2);
                    pt::time_duration timestamp = pt::time_duration(0, 0,
                        (hours.empty() ? 0 : std::stoi(hours) * 60 * 60) +
                        (minutes.empty() ? 0 : std::stoi(minutes) * 60) +
                        std::stoi(matches.str(3)), 0
                    );

                    if (timestamp > session.playingVideo->video.duration())
                    {
                        event.reply(info.settings().locale->timestampOutOfBounds(session.playingVideo->video));
                        m_logger.info(logMessage("Seek timestamp is out of bounds"));
                        return;
                    }

                    playerEntry->second.seek(timestamp.total_seconds(), info);
                    event.reply(info.settings().locale->seeking(session.playingVideo->video, timestamp, playerEntry->second.paused()));
                    m_logger.info(logMessage(fmt::format("Seeking \"{}\" to {}", session.playingVideo->video.title(), Utility::NiceString(timestamp))));
                    return;
                }
                catch (const std::out_of_range&)
                {
                    /*
                    *   Option is not a valid timestamp.
                    *   Maybe it is a chapter name?
                    */
                }
            }

            if (session.playingVideo->video.chapters().empty())
            {
                event.reply(info.settings().locale->noChapters(session.playingVideo->video));
                m_logger.info(logMessage(fmt::format("Video \"{}\" has no chapters", session.playingVideo->video.title())));
                return;
            }

            for (const Youtube::Video::Chapter& chapter : session.playingVideo->video.chapters())
            {
                if (timestampChapterOption == chapter.name)
                {
                    playerEntry->second.seek(chapter.timestamp.total_seconds(), info);
                    event.reply(info.settings().locale->seeking(
                        session.playingVideo->video,
                        chapter,
                        playerEntry->second.paused()
                    ));
                    m_logger.info(logMessage(fmt::format("Seeking \"{}\" to {}", session.playingVideo->video.title(), Utility::NiceString(chapter.timestamp))));
                    return;
                }
            }

            event.reply(info.settings().locale->unknownChapter(session.playingVideo->video));
            m_logger.info(logMessage(fmt::format("Video \"{}\" doesn't have such chapter", session.playingVideo->video.title())));
            return;
        }

        if (interaction.name == Shuffle::Name)
        {
            if (playerEntry == m_players.end())
            {
                event.reply(info.settings().locale->botNotInVoiceChannel());
                m_logger.info(logMessage("Not in voice channel"));
                return;
            }

            size_t itemCount = playerEntry->second.session().queue.size();
            switch (itemCount)
            {
                case 0:
                    event.reply(info.settings().locale->queueIsEmpty());
                    m_logger.info(logMessage("Queue is empty"));
                    return;
                case 1:
                    event.reply(info.settings().locale->cantShuffle());
                    m_logger.info(logMessage("Can't shuffle"));
                    return;
            }

            playerEntry->second.shuffle();
            event.reply(info.settings().locale->shuffled(itemCount, playerEntry->second.paused()));
            m_logger.info(logMessage(fmt::format("Shuffled {} item{}", itemCount, LocaleEn::Cardinal(itemCount))));
            return;
        }

        if (interaction.name == Skip::Name)
        {
            if (playerEntry == m_players.end())
            {
                event.reply(info.settings().locale->botNotInVoiceChannel());
                m_logger.info(logMessage("Not in voice channel"));
                return;
            }

            kc::Bot::Session session = playerEntry->second.session();
            if (!session.playingVideo)
            {
                event.reply(info.settings().locale->nothingIsPlaying());
                m_logger.info(logMessage("Not playing"));
                return;
            }

            const std::string& subcommand = interaction.options[0].name;
            if (subcommand == Skip::Video::Name)
            {
                playerEntry->second.skipVideo(info);
                event.reply(info.settings().locale->skipped(session.playingVideo->video, playerEntry->second.paused()));
                m_logger.info(logMessage(fmt::format("Skipped video \"{}\"", session.playingVideo->video.title())));
                return;
            }

            if (subcommand == Skip::Playlist::Name)
            {
                if (!session.playingPlaylist)
                {
                    event.reply(info.settings().locale->noPlaylistIsPlaying());
                    m_logger.info(logMessage("No playlist is playing"));
                    return;
                }

                playerEntry->second.skipPlaylist(info);
                event.reply(info.settings().locale->skipped(session.playingPlaylist->playlist, playerEntry->second.paused()));
                m_logger.info(logMessage(fmt::format("Skipped playlist \"{}\"", session.playingPlaylist->playlist.title())));
                return;
            }

            event.reply(info.settings().locale->unknownCommand());
            m_logger.error(logMessage(fmt::format("Unknown subcommand: \"{}\"", subcommand)));
            return;
        }

        if (interaction.name == Clear::Name)
        {
            if (playerEntry == m_players.end())
            {
                event.reply(info.settings().locale->botNotInVoiceChannel());
                m_logger.info(logMessage("Not in voice channel"));
                return;
            }

            kc::Bot::Session session = playerEntry->second.session();
            if (!session.playingVideo)
            {
                event.reply(info.settings().locale->nothingIsPlaying());
                m_logger.info(logMessage("Nothing is playing"));
                return;
            }

            if (session.queue.empty())
            {
                event.reply(info.settings().locale->queueIsEmpty());
                m_logger.info(logMessage("Queue is empty"));
                return;
            }

            playerEntry->second.clear();
            event.reply(info.settings().locale->cleared());
            m_logger.info(logMessage("Queue is cleared"));
            return;
        }

        if (interaction.name == Stop::Name)
        {
            if (playerEntry == m_players.end())
            {
                event.reply(info.settings().locale->botNotInVoiceChannel());
                m_logger.info(logMessage("Not in voice channel"));
                return;
            }

            kc::Bot::Session session = playerEntry->second.session();
            if (!session.playingVideo)
            {
                event.reply(info.settings().locale->nothingIsPlaying());
                m_logger.info(logMessage("Nothing is playing"));
                return;
            }

            playerEntry->second.stop(info);
            event.reply(info.settings().locale->stopped());
            m_logger.info(logMessage("Stopped"));
            return;
        }

        event.reply(info.settings().locale->unknownCommand());
        m_logger.error(logMessage("Unknown slashcommand"));
    });

    on_autocomplete([this](const dpp::autocomplete_t& event)
    {
        const dpp::guild& guild = event.command.get_guild();
        const Info info(guild.id);
        std::string value;

        const LogMessageFunction logMessage = [&event, &guild, &value](const std::string& message)
        {
            return fmt::format(
                "\"{}\" / \"{}\": Autocomplete for /{} \"{}\": {}",
                guild.name,
                event.command.usr.format_username(),
                event.name,
                value,
                message
            );
        };

        const dpp::command_option& option = event.options[0];
        if (option.name == Seek::TimestampChapter::Name)
        {
            value = std::get<std::string>(option.value);
            PlayerEntry playerEntry = m_players.find(guild.id);
            if (playerEntry == m_players.end())
            {
                interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
                m_logger.info(logMessage("Not in voice channel"));
                return;
            }

            kc::Bot::Session session = playerEntry->second.session();
            if (!session.playingVideo)
            {
                interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
                m_logger.info(logMessage("Nothing is playing"));
                return;
            }

            if (session.playingVideo->video.chapters().empty())
            {
                interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
                m_logger.info(logMessage(fmt::format("Video \"{}\" has no chapters", session.playingVideo->video.title())));
                return;
            }

            if (value.empty() || value.find_first_not_of(' ') == std::string::npos)
            {
                dpp::interaction_response response(dpp::ir_autocomplete_reply);
                for (size_t index = 0, size = session.playingVideo->video.chapters().size(); index < size && index < 25; ++index)
                {
                    const Youtube::Video::Chapter& chapter = session.playingVideo->video.chapters()[index];
                    std::string firstPart = fmt::format("{}: ", chapter.number);
                    std::string secondPart = fmt::format(" [{}]", Utility::NiceString(chapter.timestamp));
                    response.add_autocomplete_choice(dpp::command_option_choice(fmt::format(
                        "{}\"{}\"{}",
                        firstPart,
                        Utility::Truncate(chapter.name, 100 - 2 - firstPart.length() - secondPart.length()),
                        secondPart
                    ), chapter.name));
                }

                interaction_response_create(event.command.id, event.command.token, response);
                m_logger.info(logMessage("Displaying all chapters"));
                return;
            }

            dpp::interaction_response response(dpp::ir_autocomplete_reply);
            for (size_t index = 0, size = session.playingVideo->video.chapters().size(); index < size && response.autocomplete_choices.size() < 25; ++index)
            {
                const Youtube::Video::Chapter& chapter = session.playingVideo->video.chapters()[index];
                if (Utility::CaseInsensitiveStringContains(chapter.name, value))
                {
                    std::string firstPart = fmt::format("{}: ", chapter.number);
                    std::string secondPart = fmt::format(" [{}]", Utility::NiceString(chapter.timestamp));
                    response.add_autocomplete_choice(dpp::command_option_choice(fmt::format(
                        "{}\"{}\"{}",
                        firstPart,
                        Utility::Truncate(chapter.name, 100 - 2 - firstPart.length() - secondPart.length()),
                        secondPart
                    ), chapter.name));
                }
            }

            interaction_response_create(event.command.id, event.command.token, response);
            m_logger.info(logMessage(fmt::format(
                "Displaying {} chapter{}",
                response.autocomplete_choices.size(),
                LocaleEn::Cardinal(response.autocomplete_choices.size())
            )));
            return;
        }

        interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
        m_logger.error(logMessage(fmt::format("Unknown option: \"{}\"", option.name)));
    });

    on_button_click([this](const dpp::button_click_t& event)
    {
        const dpp::guild& guild = event.command.get_guild();
        updatePlayerTextChannelId(guild.id, event.command.channel_id);
        Info info(guild.id);
        ++info.stats().interactionsProcessed;

        const LogMessageFunction logMessage = [&event, &guild](const std::string& message)
        {
            return fmt::format("\"{}\" / \"{}\": [{}]: {}", guild.name, event.command.usr.format_username(), event.custom_id, message);
        };

        Signal signal(event.custom_id);
        switch (signal.type())
        {
            case Signal::Type::PlayVideo:
            case Signal::Type::PlayPlaylist:
            {
                if (playerControlsLocked(guild, event.command.usr.id))
                {
                    event.reply(info.settings().locale->onlyUsersWithMeCanControlPlayer());
                    m_logger.info(logMessage("Player controls are locked for user"));
                    return;
                }

                event.reply(addItem(event.from, event.command, signal.data(), logMessage, info, true));
                break;
            }
            case Signal::Type::RelatedSearch:
            {
                event.thinking(true);
                Youtube::Results relatedResult = Youtube::Related(signal.data());
                event.edit_original_response(info.settings().locale->search(relatedResult), [this, event](const dpp::confirmation_callback_t& confirmationEvent)
                {
                    if (confirmationEvent.is_error())
                        return;
                    m_ephemeralTokens[confirmationEvent.get<dpp::message>().id] = event.command.token;
                });
                m_logger.info(logMessage(fmt::format(
                    "Related for \"{}\": {} result{}",
                    signal.data(),
                    relatedResult.size(),
                    LocaleEn::Cardinal(relatedResult.size())
                )));
                return;
            }
            case Signal::Type::Unsupported:
            {
                event.reply(info.settings().locale->unsupportedButton());
                m_logger.info(logMessage("Unsupported button"));
                break;
            }
            default:
            {
                event.reply(info.settings().locale->unknownButton());
                m_logger.error(logMessage("Unknown button"));
                break;
            }
        }

        auto ephemeralTokenEntry = m_ephemeralTokens.find(event.command.msg.id);
        if (ephemeralTokenEntry == m_ephemeralTokens.end())
            return;

        dpp::message message = event.command.msg;
        for (dpp::component& component : message.components[0].components)
            component.disabled = true;

        interaction_followup_edit(ephemeralTokenEntry->second, message);
        m_ephemeralTokens.erase(event.command.msg.id); 
    });

    on_select_click([this](const dpp::select_click_t& event)
    {
        const dpp::guild& guild = event.command.get_guild();
        Info info(guild.id);
        ++info.stats().interactionsProcessed;

        const LogMessageFunction logMessage = [&event, &guild](const std::string& message)
        {
            return fmt::format(
                "\"{}\" / \"{}\": [{}/{}]: {}",
                guild.name,
                event.command.usr.format_username(),
                event.custom_id,
                event.values[0],
                message
            );
        };

        if (playerControlsLocked(guild, event.command.usr.id))
        {
            event.reply(info.settings().locale->onlyUsersWithMeCanControlPlayer());
            m_logger.info(logMessage("Player controls are locked for user"));
            return;
        }

        Signal signal(event.values[0]);
        switch (signal.type())
        {
            case Signal::Type::PlayVideo:
            case Signal::Type::PlayPlaylist:
            {
                event.reply(addItem(event.from, event.command, signal.data(), logMessage, info, true));
                break;
            }
            case Signal::Type::Unsupported:
            {
                event.reply(info.settings().locale->unsupportedButton());
                m_logger.info(logMessage("Unsupported button"));
                break;
            }
            default:
            {
                event.reply(info.settings().locale->unknownButton());
                m_logger.error(logMessage("Unknown select option"));
                break;
            }
        }

        if (m_ephemeralTokens.find(event.command.msg.id) != m_ephemeralTokens.end())
        {
            dpp::message message = event.command.msg;
            dpp::component& selectMenu = message.components[0].components[0];
            for (const dpp::select_option& option : selectMenu.options)
            {
                if (option.value == event.values[0])
                {
                    selectMenu.placeholder = option.label;
                    break;
                }
            }
            selectMenu.disabled = true;

            interaction_followup_edit(m_ephemeralTokens[message.id], message);
            m_ephemeralTokens.erase(event.command.msg.id);
        }
    });

    on_voice_ready([this](const dpp::voice_ready_t& event)
    {
        Info info(event.voice_client->server_id);
        m_players.find(event.voice_client->server_id)->second.signalReady(info);
        m_logger.info("\"{}\": Voice client is ready", dpp::find_guild(event.voice_client->server_id)->name);
    });

    on_voice_track_marker([this](const dpp::voice_track_marker_t& event)
    {
        const dpp::guild* guild = dpp::find_guild(event.voice_client->server_id);
        PlayerEntry playerEntry = m_players.find(event.voice_client->server_id);
        Info info(guild->id);

        Signal signal(event.track_meta);
        switch (signal.type())
        {
            case Signal::Type::LivestreamSkipped:
            case Signal::Type::PremiereSkipped:
            case Signal::Type::PlayError:
            case Signal::Type::Played:
            case Signal::Type::ChapterReached:
            {
                playerEntry->second.signalMarker(signal, info);
                return;
            }
            default:
            {
                m_logger.warn("Unknown voice track marker: \"{}\"", event.track_meta);
                return;
            }
        }
    });

    on_voice_state_update([this](const dpp::voice_state_update_t& event)
    {
        dpp::guild* guild = dpp::find_guild(event.state.guild_id);
        dpp::voiceconn* botVoice = event.from->get_voice(event.state.guild_id);
        Info info(guild->id);

        if (event.state.user_id != me.id)
        {
            if (botVoice && CountVoiceMembers(*guild, botVoice->channel_id) == 1)
                leaveVoice(event.from, *guild, info, Locale::EndReason::EverybodyLeft);
            return;
        }

        if (botVoice && botVoice->channel_id != event.state.channel_id)
        {
            ++info.stats().timesMoved;
            leaveVoice(event.from, *guild, info, Locale::EndReason::Moved);
            return;
        }

        if (!event.state.channel_id.empty())
            return;

        /*
        *   Player instance is deleted when bot gracefully leaves voice channel.
        *   Bot was kicked if it wasn't deleted.
        */
        PlayerEntry playerEntry = m_players.find(event.state.guild_id);
        if (playerEntry != m_players.end())
        {
            ++info.stats().timesKicked;
            playerEntry->second.endSession(info, Locale::EndReason::Kicked);
            m_players.erase(event.state.guild_id);
        }
    });

    on_voice_server_update([this](const dpp::voice_server_update_t& event)
    {
        const LogMessageFunction logMessage = [&event](const std::string& message)
        {
            return fmt::format(
                "\"{}\": {}",
                dpp::find_guild(event.guild_id)->name,
                message
            );
        };

        PlayerEntry playerEntry = m_players.find(event.guild_id);
        if (playerEntry == m_players.end())
        {
            m_logger.warn(logMessage("Voice server update event received from guild with no player"));
            return;
        }

        std::string playerEndpoint = playerEntry->second.session().voiceServerEndpoint;
        if (playerEndpoint.empty())
        {
            playerEntry->second.updateVoiceServerEndpoint(event.endpoint);
            return;
        }

        if (playerEndpoint == event.endpoint)
            return;
        playerEntry->second.updateVoiceServerEndpoint(event.endpoint);

        dpp::voiceconn* connection = event.from->get_voice(event.guild_id);
        connection->disconnect();
        connection->websocket_hostname = event.endpoint;
        connection->connect(event.guild_id);
        m_logger.warn(logMessage(fmt::format("Voice server changed from \"{}\" to \"{}\", reconnecting", playerEndpoint, event.endpoint)));
    });
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
