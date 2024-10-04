#include "bot/bot.hpp"

namespace kc {

void Bot::Bot::onSlashcommand(const dpp::slashcommand_t& event)
{
    const dpp::command_interaction interaction = event.command.get_command_interaction();
    const dpp::guild& guild = event.command.get_guild();
    const LogMessageFunction logMessage = [event, interaction, guild](const std::string& message)
    {
        if (!interaction.options.empty())
        {
            if (interaction.name == CommandsConst::Play::Name || interaction.name == CommandsConst::Seek::Name)
            {
                return fmt::format(
                    "\"{}\" / \"{}\": /{} \"{}\": {}",
                    guild.name, event.command.usr.format_username(),
                    interaction.name, std::get<std::string>(interaction.options[0].value), message
                );
            }

            if (interaction.name == CommandsConst::Skip::Name)
            {
                return fmt::format(
                    "\"{}\" / \"{}\": /{} {}: {}",
                    guild.name, event.command.usr.format_username(),
                    interaction.name, interaction.options[0].name, message
                );
            }

            if (interaction.name == CommandsConst::Set::Name)
            {
                if (interaction.options[0].options[0].name == CommandsConst::Set::Language::Language::Name ||
                    interaction.options[0].options[0].name == CommandsConst::Set::ChangeStatus::Change::Name)
                {
                    return fmt::format(
                        "\"{}\" / \"{}\": /{} {} \"{}\": {}",
                        guild.name, event.command.usr.format_username(),
                        interaction.name, interaction.options[0].name, std::get<std::string>(interaction.options[0].options[0].value),
                        message
                    );
                }

                if (interaction.options[0].options[0].name == CommandsConst::Set::Timeout::Duration::Name)
                {
                    return fmt::format(
                        "\"{}\" / \"{}\": /{} {} {}: {}",
                        guild.name, event.command.usr.format_username(),
                        interaction.name, interaction.options[0].name, std::get<int64_t>(interaction.options[0].options[0].value),
                        message
                    );
                }
            }
        }

        return fmt::format(
            "\"{}\" / \"{}\": /{}: {}",
            guild.name, event.command.usr.format_username(),
            interaction.name, message
        );
    };

    std::lock_guard lock(m_mutex);
    PlayerEntry playerEntry = updatePlayerTextChannelId(guild.id, event.command.channel_id);
    Info info = updateInfoProcessedInteractions(guild.id);

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

    if (interaction.name == CommandsConst::Set::Name)
    {
        const std::string& subcommand = interaction.options[0].name;
        if (subcommand == CommandsConst::Set::Language::Name)
        {
            info.settings().locale = Locale::Create(std::get<std::string>(interaction.options[0].options[0].value));
            if (playerEntry != m_players.end())
                playerEntry->second.updateVoiceStatus(info);
            event.reply(info.settings().locale->soBeIt());
            m_logger.info(logMessage(fmt::format("Locale set to \"{}\"", info.settings().locale->name())));
            return;
        }

        if (subcommand == CommandsConst::Set::Timeout::Name)
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

        if (subcommand == CommandsConst::Set::ChangeStatus::Name)
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

    if (interaction.name == CommandsConst::Join::Name)
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

    if (interaction.name == CommandsConst::Leave::Name)
    {
        std::optional<Session> session;
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

    if (interaction.name == CommandsConst::Play::Name)
    {
        const std::string& whatOption = std::get<std::string>(interaction.options[0].value);
        boost::smatch matches, videoMatches, playlistMatches;
        if (boost::regex_search(whatOption, videoMatches, boost::regex(Youtube::VideoConst::ExtractId)) &&
            boost::regex_search(whatOption, playlistMatches, boost::regex(Youtube::PlaylistConst::ExtractId)))
        {
            event.reply(info.settings().locale->ambiguousPlay(videoMatches.str(1), playlistMatches.str(1)));
            event.get_original_response(std::bind(&Bot::updateEphemeralToken, this, std::placeholders::_1, event.command.token));
            m_logger.info(logMessage("Ambiguous"));
            return;
        }

        std::thread([this, event, guild, logMessage, whatOption]()
        {
            if (boost::regex_search(whatOption, boost::regex(Youtube::VideoConst::ExtractId)) ||
                boost::regex_search(whatOption, boost::regex(Youtube::PlaylistConst::ExtractId)))
            {
                event.thinking();
                event.edit_original_response(addItem(event.from, event.command, whatOption, logMessage));
                return;
            }

            try
            {
                event.thinking(true);
                Youtube::Results results = Youtube::Search(whatOption);
                std::lock_guard lock(m_mutex);

                event.edit_original_response(
                    Info(guild.id).settings().locale->search(results),
                    std::bind(&Bot::updateEphemeralToken, this, std::placeholders::_1, event.command.token)
                );
                m_logger.info(logMessage(fmt::format("{} result{}", results.size(), LocaleEn::Cardinal(results.size()))));
            }
            catch (const std::runtime_error& error)
            {
                std::lock_guard lock(m_mutex);
                event.edit_original_response(Info(guild.id).settings().locale->unknownError());
                m_logger.error(logMessage(fmt::format("Runtime error: {}", error.what())));
            }
        }).detach();
        return;
    }

    if (interaction.name == CommandsConst::Pause::Name)
    {
        if (playerEntry == m_players.end())
        {
            event.reply(info.settings().locale->botNotInVoiceChannel());
            m_logger.info(logMessage("Not in voice channel"));
            return;
        }

        Session session = playerEntry->second.session();
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

    if (interaction.name == CommandsConst::Seek::Name)
    {
        if (playerEntry == m_players.end())
        {
            event.reply(info.settings().locale->botNotInVoiceChannel());
            m_logger.info(logMessage("Not in voice channel"));
            return;
        }

        Session session = playerEntry->second.session();
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

    if (interaction.name == CommandsConst::Shuffle::Name)
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

    if (interaction.name == CommandsConst::Skip::Name)
    {
        if (playerEntry == m_players.end())
        {
            event.reply(info.settings().locale->botNotInVoiceChannel());
            m_logger.info(logMessage("Not in voice channel"));
            return;
        }

        Session session = playerEntry->second.session();
        if (!session.playingVideo)
        {
            event.reply(info.settings().locale->nothingIsPlaying());
            m_logger.info(logMessage("Not playing"));
            return;
        }

        const std::string& subcommand = interaction.options[0].name;
        if (subcommand == CommandsConst::Skip::Video::Name)
        {
            playerEntry->second.skipVideo(info);
            event.reply(info.settings().locale->skipped(session.playingVideo->video, playerEntry->second.paused()));
            m_logger.info(logMessage(fmt::format("Skipped video \"{}\"", session.playingVideo->video.title())));
            return;
        }

        if (subcommand == CommandsConst::Skip::Playlist::Name)
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

    if (interaction.name == CommandsConst::Clear::Name)
    {
        if (playerEntry == m_players.end())
        {
            event.reply(info.settings().locale->botNotInVoiceChannel());
            m_logger.info(logMessage("Not in voice channel"));
            return;
        }

        Session session = playerEntry->second.session();
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

    if (interaction.name == CommandsConst::Stop::Name)
    {
        if (playerEntry == m_players.end())
        {
            event.reply(info.settings().locale->botNotInVoiceChannel());
            m_logger.info(logMessage("Not in voice channel"));
            return;
        }

        Session session = playerEntry->second.session();
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
}

} // namespace kc
