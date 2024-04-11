﻿#include "bot/bot.hpp"

namespace kc {

size_t Bot::Bot::CountVoiceMembers(const dpp::guild& guild, dpp::snowflake channelId)
{
    size_t count = 0;
    for (const auto& voiceMember : guild.voice_members)
        count += (voiceMember.second.channel_id == channelId);
    return count;
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
            }

            m_logger.info(logMessage(fmt::format(
                "Added \"{}\" / \"{}\" [{}]",
                video.author(),
                video.title(),
                Utility::NiceString(video.duration())
            )));

            if (showRequester)
                return info.settings().locale->itemAdded(video, playerEntry->second.paused(), interaction.get_issuing_user());
            return info.settings().locale->itemAdded(video, playerEntry->second.paused());
        }

        Youtube::Playlist playlist(itemId);
        JoinStatus joinStatus = joinUserVoice(client, interaction, info, playlist);
        PlayerEntry playerEntry = m_players.find(guild.id);
        switch (joinStatus.result)
        {
            case JoinStatus::Result::AlreadyJoined:
                playerEntry->second.addItem(playlist, requester, info);
                break;
            case JoinStatus::Result::CantJoin:
                m_logger.info(logMessage("Can't join"));
                return info.settings().locale->cantJoin();
        }

        m_logger.info(logMessage(fmt::format("Added \"{}\" / \"{}\" [{} videos]", playlist.author(), playlist.title(), Utility::NiceString(playlist.videoCount()))));
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
    catch (...)
    {
        m_logger.error(logMessage("Unknown error"));
        return info.settings().locale->unknownError();
    }
}

Bot::Bot::Bot(std::shared_ptr<Config> config, bool registerCommands)
    : cluster(config->discordBotApiToken())
    , m_logger("bot", std::make_shared<spdlog::sinks::stdout_color_sink_mt>())
{
    using namespace Commands;

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

                /* /session */
                commands.push_back(dpp::slashcommand(Commands::Session::Name, Commands::Session::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Session::Name, Russian::Session::Description));

                /* /settings */
                commands.push_back(dpp::slashcommand(Commands::Settings::Name, Commands::Settings::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Settings::Name, Russian::Settings::Description));

                /* /stats */
                commands.push_back(dpp::slashcommand(Commands::Stats::Name, Commands::Stats::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Stats::Name, Russian::Stats::Description));

                /* /set language, /set timeout, /set change-status */
                commands.push_back(dpp::slashcommand(Set::Name, "-", me.id)
                    .add_localization(Russian::LocaleName, Russian::Set::Name, "-")
                    .add_option(dpp::command_option(dpp::co_sub_command, Set::Language::Name, Set::Language::Description)
                        .add_localization(Russian::LocaleName, Russian::Set::Language::Name, Russian::Set::Language::Description)
                        .add_option(dpp::command_option(dpp::co_string, Set::Language::Language::Name, Set::Language::Language::Description, true)
                            .add_localization(Russian::LocaleName, Russian::Set::Language::Language::Name, Russian::Set::Language::Language::Description)
                            .add_choice(dpp::command_option_choice(Set::Language::Language::Label, std::string(Set::Language::Language::Id)))
                            .add_choice(dpp::command_option_choice(Russian::Set::Language::Language::Label, std::string(Russian::Set::Language::Language::Id)))))
                    .add_option(dpp::command_option(dpp::co_sub_command, Set::Timeout::Name, Set::Timeout::Description)
                        .add_localization(Russian::LocaleName, Russian::Set::Timeout::Name, Russian::Set::Timeout::Description)
                        .add_option(dpp::command_option(dpp::co_integer, Set::Timeout::Duration::Name, Set::Timeout::Duration::Description, true)
                            .add_localization(Russian::LocaleName, Russian::Set::Timeout::Duration::Name, Russian::Set::Timeout::Duration::Description)))
                    .add_option(dpp::command_option(dpp::co_sub_command, Set::ChangeStatus::Name, Set::ChangeStatus::Description)
                       .add_localization(Russian::LocaleName, Russian::Set::ChangeStatus::Name, Russian::Set::ChangeStatus::Description)
                       .add_option(dpp::command_option(dpp::co_boolean, Set::ChangeStatus::Change::Name, Set::ChangeStatus::Change::Description, true)
                           .add_localization(Russian::LocaleName, Russian::Set::ChangeStatus::Change::Name, Russian::Set::ChangeStatus::Change::Description))));

                /* /join */
                commands.push_back(dpp::slashcommand(Join::Name, Join::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Join::Name, Russian::Join::Description));

                /* /leave */
                commands.push_back(dpp::slashcommand(Leave::Name, Leave::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Leave::Name, Russian::Leave::Description));

                /* /play <what> */
                commands.push_back(dpp::slashcommand(Play::Name, Play::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Play::Name, Russian::Play::Description)
                    .add_option(dpp::command_option(dpp::co_string, Play::What::Name, Play::What::Description, true)
                        .add_localization(Russian::LocaleName, Russian::Play::What::Name, Russian::Play::What::Description)
                        .set_auto_complete(true)));

                /* /pause */
                commands.push_back(dpp::slashcommand(Pause::Name, Pause::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Pause::Name, Russian::Pause::Description));

                /* /seek <timestamp-chapter> */
                commands.push_back(dpp::slashcommand(Seek::Name, Seek::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Seek::Name, Russian::Seek::Description)
                    .add_option(dpp::command_option(dpp::co_string, Seek::TimestampChapter::Name, Seek::TimestampChapter::Description, true)
                        .add_localization(Russian::LocaleName, Russian::Seek::TimestampChapter::Name, Russian::Seek::TimestampChapter::Description)
                        .set_auto_complete(true)));

                /* /shuffle */
                commands.push_back(dpp::slashcommand(Shuffle::Name, Shuffle::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Shuffle::Name, Russian::Shuffle::Description));

                /* /skip video, /skip playlist */
                commands.push_back(dpp::slashcommand(Skip::Name, "-", me.id)
                    .add_localization(Russian::LocaleName, Russian::Skip::Name, "-")
                    .add_option(dpp::command_option(dpp::co_sub_command, Skip::Video::Name, Skip::Video::Description)
                        .add_localization(Russian::LocaleName, Russian::Skip::Video::Name, Russian::Skip::Video::Description))
                    .add_option(dpp::command_option(dpp::co_sub_command, Skip::Playlist::Name, Skip::Playlist::Description)
                        .add_localization(Russian::LocaleName, Russian::Skip::Playlist::Name, Russian::Skip::Playlist::Description)));

                /* /clear */
                commands.push_back(dpp::slashcommand(Clear::Name, Clear::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Clear::Name, Russian::Clear::Description));

                /* /stop */
                commands.push_back(dpp::slashcommand(Stop::Name, Stop::Description, me.id)
                    .add_localization(Russian::LocaleName, Russian::Stop::Name, Russian::Stop::Description));

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
                    if (interaction.options[0].options[0].name == Set::Language::Language::Name)
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

                    if (interaction.options[0].options[0].name == Set::ChangeStatus::Change::Name)
                    {
                        return fmt::format(
                            "\"{}\" / \"{}\": /{} {} {}: {}",
                            guild.name,
                            event.command.usr.format_username(),
                            interaction.name,
                            interaction.options[0].name,
                            std::get<bool>(interaction.options[0].options[0].value),
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

        if (interaction.name == Commands::Session::Name)
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

        if (interaction.name == Commands::Settings::Name)
        {
            event.reply(info.settings().locale->settings(info.settings()));
            m_logger.info(logMessage("Showing guild's settings"));
            return;
        }

        if (interaction.name == Commands::Stats::Name)
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
                event.reply(info.settings().locale->timeoutDurationSet(timeoutDuration));
                m_logger.info(logMessage(fmt::format("Timeout duration is set to {}", Utility::NiceString(pt::time_duration(0, timeoutDuration, 0)))));
                return;
            }

            if (subcommand == Set::ChangeStatus::Name)
            {
                info.settings().changeStatus = std::get<bool>(interaction.options[0].options[0].value);
                if (playerEntry != m_players.end())
                    playerEntry->second.updateVoiceStatus(info);
                event.reply(info.settings().locale->soBeIt());
                m_logger.info(logMessage(fmt::format("Change status set to {}", info.settings().changeStatus)));
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
            }
        }

        if (playerControlsLocked(guild, event.command.usr.id))
        {
            event.reply(info.settings().locale->alreadyTaken());
            m_logger.info(logMessage("Player controls are locked for user"));
            return;
        }

        if (interaction.name == Leave::Name)
        {
            std::optional<kc::Bot::Session> session;
            if (playerEntry != m_players.end())
            {
                playerEntry->second.endSession(info);
                session.emplace(playerEntry->second.session());
            }

            LeaveStatus leaveStatus = leaveVoice(event.from, guild, info);
            switch (leaveStatus.result)
            {
                case LeaveStatus::Result::Left:
                {
                    dpp::message message = info.settings().locale->left(leaveStatus.channel->id);
                    message.add_embed(info.settings().locale->sessionEnd(Locale::EndReason::UserRequested, *session).embeds[0]);
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
                        event.reply(info.settings().locale->timestampOutOfBounds(session.playingVideo->video.title(), session.playingVideo->video.duration()));
                        m_logger.info(logMessage("Seek timestamp is out of bounds"));
                        return;
                    }

                    playerEntry->second.seek(timestamp.total_seconds(), info);
                    event.reply(info.settings().locale->seeking(session.playingVideo->video.title(), timestamp, playerEntry->second.paused()));
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
                event.reply(info.settings().locale->noChapters(session.playingVideo->video.title()));
                m_logger.info(logMessage(fmt::format("Video \"{}\" has no chapters", session.playingVideo->video.title())));
                return;
            }

            for (const Youtube::Video::Chapter& chapter : session.playingVideo->video.chapters())
            {
                if (timestampChapterOption == chapter.name)
                {
                    playerEntry->second.seek(chapter.timestamp.total_seconds(), info);
                    event.reply(info.settings().locale->seeking(
                        playerEntry->second.session().playingVideo->video.title(),
                        chapter.name,
                        chapter.timestamp,
                        playerEntry->second.paused()
                    ));
                    m_logger.info(logMessage(fmt::format("Seeking \"{}\" to {}", session.playingVideo->video.title(), Utility::NiceString(chapter.timestamp))));
                    return;
                }
            }

            event.reply(info.settings().locale->unknownChapter(session.playingVideo->video.title()));
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
                    response.add_autocomplete_choice(dpp::command_option_choice(fmt::format(
                        "{}: {} [{}]",
                        chapter.number,
                        chapter.name,
                        Utility::NiceString(chapter.timestamp)
                    ), chapter.name));
                }

                interaction_response_create(event.command.id, event.command.token, response);
                m_logger.info(logMessage("Displaying all chapters"));
                return;
            }

            dpp::interaction_response response(dpp::ir_autocomplete_reply);
            for (size_t index = 0, size = session.playingVideo->video.chapters().size(); index < size && response.autocomplete_choices.size() < 25; ++index)
            {
                const Youtube::Video::Chapter& currentChapter = session.playingVideo->video.chapters()[index];
                if (Utility::CaseInsensitiveStringContains(currentChapter.name, value))
                {
                    response.add_autocomplete_choice(dpp::command_option_choice(fmt::format(
                        "{}: \"{}\" [{}]",
                        index + 1,
                        currentChapter.name,
                        Utility::NiceString(currentChapter.timestamp)
                    ), currentChapter.name));
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

        if (option.name == Play::What::Name)
        {
            value = std::get<std::string>(option.value);
            if (value.empty() || value.find_first_not_of(' ') == std::string::npos)
            {
                interaction_response_create(event.command.id, event.command.token, dpp::interaction_response(dpp::ir_autocomplete_reply));
                m_logger.info(logMessage("Option is empty"));
                return;
            }

            try
            {
                boost::smatch videoMatches;
                bool videoMatched = boost::regex_search(value, videoMatches, boost::regex(Youtube::VideoConst::ExtractId));
                if (!videoMatched)
                    videoMatched = boost::regex_match(value, videoMatches, boost::regex(Youtube::VideoConst::ValidateId));

                boost::smatch playlistMatches;
                bool playlistMatched = boost::regex_search(value, playlistMatches, boost::regex(Youtube::PlaylistConst::ExtractId));
                if (!playlistMatched)
                    playlistMatched = boost::regex_match(value, playlistMatches, boost::regex(Youtube::PlaylistConst::ValidateId));

                if (videoMatched && playlistMatched)
                {
                    Youtube::Video video(videoMatches.str(1));
                    Youtube::Playlist playlist(playlistMatches.str(1));

                    dpp::interaction_response response(dpp::ir_autocomplete_reply);
                    response.add_autocomplete_choice(info.settings().locale->itemAutocomplete(video));
                    response.add_autocomplete_choice(info.settings().locale->itemAutocomplete(playlist));

                    interaction_response_create(event.command.id, event.command.token, response);
                    m_logger.info(logMessage("Displaying video and playlist suggestions"));
                    return;
                }

                if (videoMatched)
                {
                    Youtube::Video video(videoMatches.str(1));
                    dpp::interaction_response response(dpp::ir_autocomplete_reply);
                    response.add_autocomplete_choice(info.settings().locale->itemAutocomplete(video));

                    interaction_response_create(event.command.id, event.command.token, response);
                    m_logger.info(logMessage("Displaying video suggestion"));
                    return;
                }

                if (playlistMatched)
                {
                    Youtube::Playlist playlist(playlistMatches.str(1));
                    dpp::interaction_response response(dpp::ir_autocomplete_reply);
                    response.add_autocomplete_choice(info.settings().locale->itemAutocomplete(playlist));

                    interaction_response_create(event.command.id, event.command.token, response);
                    m_logger.info(logMessage("Displaying playlist suggestion"));
                    return;
                }
            }
            catch (...)
            {
                /*
                *   Some error happened when generating video or playlist suggestions.
                *   We'll proceed with search with option as query.
                */
            }

            dpp::interaction_response response(dpp::ir_autocomplete_reply);
            Youtube::Results result = Youtube::Search(value);
            for (size_t index = 0, size = result.size(); index < size && index < 25; ++index)
                response.add_autocomplete_choice(info.settings().locale->itemAutocomplete(result[index]));

            interaction_response_create(event.command.id, event.command.token, response);
            m_logger.info(logMessage(fmt::format(
                "{} result{}",
                result.size(),
                LocaleEn::Cardinal(result.size())
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
            return fmt::format("\"{}\" / \"{}\": [{}]: {}", event.command.usr.format_username(), guild.name, event.custom_id, message);
        };

        Signal signal(event.custom_id);
        switch (signal.type())
        {
            case Signal::Type::PlayVideo:
            case Signal::Type::PlayPlaylist:
            {
                if (playerControlsLocked(guild, event.command.usr.id))
                {
                    event.reply(info.settings().locale->alreadyTaken());
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
                event.command.usr.format_username(),
                guild.name, event.custom_id,
                event.values[0],
                message
            );
        };

        if (playerControlsLocked(guild, event.command.usr.id))
        {
            event.reply(info.settings().locale->alreadyTaken());
            m_logger.info(logMessage("Player controls are locked for user"));
            return;
        }

        Signal signal(event.values[0]);
        switch (signal.type())
        {
            case Signal::Type::PlayVideo:
            case Signal::Type::PlayPlaylist:
                event.reply(addItem(event.from, event.command, signal.data(), logMessage, info, true));
                break;
            default:
                event.reply(info.settings().locale->unknownOption());
                m_logger.error(logMessage(fmt::format("Unknown select option: {}", event.values[0])));
                break;
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
}

Bot::Bot::LeaveStatus Bot::Bot::leaveVoice(dpp::discord_client* client, const dpp::guild& guild, Info& info, Locale::EndReason reason)
{
    dpp::voiceconn* botVoice = client->get_voice(guild.id);
    if (!botVoice)
        return { LeaveStatus::Result::BotNotInVoiceChannel };

    const dpp::channel* disconnectedChannel = dpp::find_channel(botVoice->channel_id);
    if (reason != Locale::EndReason::UserRequested)
        m_players.find(guild.id)->second.endSession(info, reason);
    m_players.erase(guild.id);
    client->disconnect_voice(guild.id);
    return { LeaveStatus::Result::Left, disconnectedChannel };
}

} // namespace kc