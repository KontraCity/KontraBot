#pragma once

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale.hpp"
#include "core/utility.hpp"

namespace kb {

namespace Bot
{
    class LocaleEn : public Locale
    {
    public:
        static constexpr Locale::Type Type = Locale::Type::English;
        static constexpr const char* Name = "en";
        static constexpr const char* LongName = "English";

    public:
        /// @brief Get number's cardinal ending
        /// @param number The number in question
        /// @return Number's cardinal ending
        static inline const char* Cardinal(size_t number)
        {
            if (number == 1)
                return "";
            return "s";
        }

    public:
        /// @brief Get locale type
        /// @return Locale type
        virtual inline Locale::Type type()
        {
            return Type;
        }

        /// @brief Get locale name
        /// @return Locale name
        virtual inline const char* name()
        {
            return Name;
        }

        /// @brief Get long locale name
        /// @return Long locale name
        virtual inline const char* longName()
        {
            return LongName;
        }

        /// @brief Create mention reply message
        /// @return Mention reply message
        virtual inline dpp::message mention()
        {
            MentionReplyStrings strings = {};
            strings.ifYouNeedAnyHelp = "**If you need any help about how I play music, use {}.**";
            strings.differentLanguagesHint = {
                ">>> ***Я могу разговаривать на разных языках!***\n"
                "Сейчас это **`Английский`**.\n"
                "Используй {}, если тебе надо его поменять."
            };
            return MentionReplyMessage(strings);
        }

        /// @brief Create help message
        /// @return Ephemeral message
        virtual inline dpp::message help()
        {
            HelpStrings strings = {};
            strings.hereAreAllOfMyCommands = "Here are all of my commands";
            strings.differentLanguagesHint = {
                ">>> ***Я могу разговаривать на разных языках!***\n"
                "Сейчас это **`Английский`**.\n"
                "Используй {}, если тебе надо его поменять."
            };
            strings.helpDescription = "Show this help message.";
            strings.sessionDescription = "Show information about current session: playing video and its chapters, playing playlist and its videos, videos/playlists in queue and more.";
            strings.sessionFaq = {
                ">>> ***What is a session?***\n"
                "A session is a state where I am sitting in a voice channel. It starts when I join and ends when I leave the channel."
            };
            strings.settingsDescription = "Show settings of this guild: language, inactivity timeout duration and more.";
            strings.statsDescription = "Show statistics of this guild: count of commands issued, buttons clicked, sessions conducted, tracks played and more.";
            strings.setDescription = "Configure settings of this guild: language, inactivity timeout duration and whether I should change voice channel status or not.";
            strings.setFaq = {
                ">>> ***What is an inactivity timeout?***\n"
                "Sooner or later the queue is going to run out of videos/playlists to play and I'm going to sit and do nothing. "
                "This setting defines how much time should pass before I leave the voice channel if I'm inactive.\n"
                "***What is a voice channel status?***\n"
                "That's the label just below the channel name. For me it's the easiest and most informative way to let everyone know what's currently playing. "
                "It can be banned if voice channel status is used for other purposes in this guild and you don't want me to change it."
            };
            strings.joinDescription = "Join your voice channel. Don't forget that you should join it yourself first.";
            strings.leaveDescription = "Leave voice channel that I'm currently sitting in.";
            strings.playDescription = "Play video or playlist from YouTube. Your request will be added to queue if something is already playing.";
            strings.playFaq = {
                ">>> ***What and how can be played?***\n"
                "Almost everything can be played, even YouTube #Shorts.\n"
                "*However, unfortunately, I don't know how to play streams and premieres yet.*\n"
                "Feel free to send a link of the item you want to play or start typing its title, so I can search YouTube and help you by showing suggestions."
            };
            strings.pauseDescription = "Pause currently playing video or resume playing it if it is already paused.";
            strings.seekDescription = "Seek playing video to timestamp or chapter.";
            strings.seekFaq = {
                ">>> ***How to seek to timestamp?***\n"
                "Just write the timestamp in seconds (`50`, `743`, `9965`) or with colons (`50`, `12:23`, `2:46:05`).\n"
                "*If what's written doesn't look like a timestamp, I'll interpret it as a chapter name.*\n"
                "***How to seek to chapter?***\n"
                "This kind of seek is only possible if the video has chapters. Start typing the name of the desired chapter, so that I can filter them and show you the matched ones.\n"
                "*I'll show a list of all chapters for an empty query, but due to Discord limitations, there will be at max 25 of them there.*"
            };
            strings.shuffleDescription = "Shuffle videos/playlists in queue. Doesn't do anything to videos in playing playlist.";
            strings.skipDescription = "Skip playing video or whole playlist.";
            strings.clearDescription = "Clear videos/playlists in queue. Doesn't do anything to playing video/playlist.";
            strings.stopDescription = "Stop playback of playing video/playlist and clear queue.";
            return HelpMessage(strings);
        }

        /// @brief Create "I am not sitting in any voice channel" message
        /// @return Ephemeral message
        virtual inline dpp::message botNotInVoiceChannel()
        {
            return ProblemMessage("I am not sitting in any voice channel");
        }

        /// @brief Create session message
        /// @param session Player session
        /// @return Ephemeral message
        virtual inline dpp::message session(const Session& session)
        {
            SessionStrings strings = {};
            strings.infoFooter = "Session lasted {} | Tracks played: {}";
            strings.prettyQuiet = "It's pretty quiet here";
            strings.nothingIsPlaying = "Nothing is playing. Go ahead and add something to queue!";
            strings.video = "Video";
            strings.chapter = "Chapter";
            strings.videoInfo = "by {}, [{}]";
            strings.requestedBy = "Requested by **{}**";
            strings.lastVideoPlaylistInfo = {
                "Playlist by {}\n"
                "Last video is playing"
            };
            strings.playlistInfo = {
                "Playlist by {}\n"
                "{} video{} left"
            };
            strings.morePlaylistVideos = "... {} more video{}";
            strings.playlistVideoInfo = "Video by {}, [{}]";
            strings.playlistLivestreamInfo = {
                "Livestream by {}\n"
                "> Will be skipped because I can't play them!"
            };
            strings.playlistPremiereInfo = {
                "Premiere by {}\n"
                "> Will be skipped because I can't play them!"
            };
            strings.queueIsEmpty = "Queue is empty";
            strings.queueInfo = "Queue: {} item{}";
            strings.moreQueueItems = "... {} more item{}";
            strings.queueVideoInfo = "Video by {}, [{}]";
            strings.queuePlaylistInfo = "Playlist by {}, [{} video{}]";
            return SessionMessage(strings, &Cardinal, session);
        }

        /// @brief Create guild settings message
        /// @param settings Guild settings
        /// @return Normal message
        virtual inline dpp::message settings(const Settings& settings)
        {
            SettingsStrings strings = {};
            strings.hereAreTheSettings = "Here are this guild's settings";
            strings.language = "Language";
            strings.timeoutDuration = "Timeout duration";
            strings.changeStatus = "Allowed to change voice channel status";
            strings.yes = "Yes";
            strings.no = "No";
            return SettingsMessage(strings, settings);
        }

        /// @brief Create guild stats message
        /// @param stats Guild stats
        /// @return Normal message
        virtual inline dpp::message stats(const Stats& stats)
        {
            StatsStrings strings = {};
            strings.hereAreTheStats = "Here are this guild's stats";
            strings.interactionsProcessed = "Commands issued and buttons clicked";
            strings.sessionsConducted = "Sessions conducted";
            strings.tracksPlayed = "Tracks played";
            strings.timesKicked = "Times I was kicked";
            strings.timesMoved = "Times I was moved";
            return StatsMessage(strings, stats);
        }

        /// @brief Create "Only users sitting with me can control the player" message
        /// @return Ephemeral message
        virtual inline dpp::message onlyUsersWithMeCanControlPlayer()
        {
            return ProblemMessage("Only users sitting with me can control the player");
        }

        /// @brief Randomly create "So be it" or "Whatever you want" message
        /// @return Normal message
        virtual inline dpp::message soBeIt()
        {
            switch (Utility::RandomNumber(0, 4))
            {
                default:
                    return SuccessMessage("So be it");
                case 1:
                    return SuccessMessage("Whatever you want");
                case 2:
                    return SuccessMessage("Alright");
                case 3:
                    return SuccessMessage("Carefully recorded");
                case 4:
                    return SuccessMessage("Ok");
            }
        }

        /// @brief Create "Inactivity timeout duration must be greater than zero" message
        /// @return Ephemeral message
        virtual inline dpp::message badTimeoutDuration()
        {
            return ProblemMessage("Inactivity timeout duration must be greater than zero");
        }

        /// @brief Create "Sorry, I'm not ready to sit and do nothing for more than two hours" message
        /// @return Ephemeral message
        virtual inline dpp::message longTimeoutDuration()
        {
            return ProblemMessage("Sorry, I'm not ready to sit and do nothing for more than two hours");
        }

        /// @brief Create "Inactivity timeout duration is set to <duration>" message
        /// @param timeoutMinutes Duration of timeout in minutes
        /// @return Ephemeral message
        virtual inline dpp::message timeoutDurationSet(uint64_t timeoutMinutes)
        {
            return SuccessMessage(fmt::format(
                "Inactivity timeout duration is set to `{}`",
                Utility::NiceString(pt::time_duration(0, timeoutMinutes, 0))
            ));
        }

        /// @brief Create "Something went wrong, I don't recognize this command" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownCommand()
        {
            return ErrorMessage("Something went wrong, I don't recognize this command");
        }

        /// @brief Create "Joining <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Normal message
        virtual inline dpp::message joining(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format("Joining <#{}>", static_cast<uint64_t>(channelId)));
        }

        /// @brief Create "Already joined <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Ephemeral message
        virtual inline dpp::message alreadyJoined(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format("Already joined <#{}>", static_cast<uint64_t>(channelId))).set_flags(dpp::m_ephemeral);
        }

        /// @brief Create "I'm already sitting in another voice channel" message
        /// @param channelId ID of voice channel in question
        /// @return Ephemeral message
        virtual inline dpp::message cantJoin()
        {
            return ProblemMessage("I'm already sitting in another voice channel");
        }

        /// @brief Create "Join a voice channel first" message
        /// @return Ephemeral message
        virtual inline dpp::message userNotInVoiceChannel()
        {
            return ProblemMessage("Join a voice channel first");
        }

        /// @brief Crate "I can't play in AFK channels!" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayInAfkChannels()
        {
            return ProblemMessage("I can't play in AFK channels!");
        }

        /// @brief Create "Left <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Normal message
        virtual inline dpp::message left(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format("Left <#{}>", static_cast<uint64_t>(channelId)));
        }

        /// @brief Create ambiguous play message
        /// @param videoId ID of video to play
        /// @param playlistId ID of playlist to play
        /// @return Ephemeral message
        virtual inline dpp::message ambiguousPlay(const std::string& videoId, const std::string& playlistId)
        {
            AmbigousPlayStrings strings = {};
            strings.playPlaylistOrVideo = "Play whole playlist or just this video?";
            strings.playVideo = "Play video";
            strings.playPlaylist = "Play playlist";
            return AmbiguousPlayMessage(strings, videoId, playlistId);
        }

        /// @brief Create "Sorry, I can't play livestreams" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayLivestreams()
        {
            return ProblemMessage("Sorry, I can't play livestreams");
        }

        /// @brief Create "Sorry, I can't play premieres" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayPremieres()
        {
            return ProblemMessage("Sorry, I can't play premieres");
        }

        virtual inline dpp::message cantPlayEmptyPlaylists() {
            return ProblemMessage("This playlist is empty");
        }

        virtual inline dpp::message temporarilyUnsupported() {
            return GenericMessage(
                LocaleConst::Colors::TemporarilyUnsupported,
                LocaleConst::Emojis::TemporarilyUnsupported,
                "Sorry, this is temporarily unsupported", true
            );
        }

        /// @brief Create youtube error message
        /// @param error Error in question
        /// @return Ephemeral message
        virtual inline dpp::message youtubeError(const ytcpp::YtError& error)
        {
            switch (error.type())
            {
                case ytcpp::YtError::Type::Private:
                    return ProblemMessage("This video is private");
                case ytcpp::YtError::Type::Unplayable:
                    return ProblemMessage("This video is unplayable");
                case ytcpp::YtError::Type::Unavailable:
                    return ProblemMessage("This video is unavailable");
                default:
                    return ProblemMessage("YouTube refused to let me play this item");
            }
        }

        /// @brief Create unknown error message
        /// @return Ephemeral message
        virtual inline dpp::message unknownError()
        {
            return ErrorMessage("Sorry, something went wrong");
        }

        /// @brief Create "Item added to queue" message
        /// @param item Added item
        /// @param paused Whether or not to display paused player warning
        /// @param requester User that added the item if needs to be shown in message
        /// @throw std::runtime_error if item type is unknown
        /// @return Normal message
        virtual inline dpp::message itemAdded(const ytcpp::Item& item, bool paused, const std::optional<dpp::user>& requester = {})
        {
            ItemAddedStrings strings = {};
            strings.requestedBy = "Requested by {}";
            strings.videoAdded = "Video added to queue";
            strings.playlistAdded = "Playlist added to queue";
            strings.playlistInfo = "{} [{} video{}]";
            strings.playAgain = "Play again";
            strings.related = "Related";
            strings.youtube = "YouTube";

            dpp::message message = ItemAddedMessage(strings, item, &Cardinal, requester);
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create search message
        /// @param results Search results
        /// @return Ephemeral message
        virtual inline dpp::message search(const ytcpp::SearchResults& results)
        {
            SearchStrings strings = {};
            strings.noResults = "No results";
            strings.resultCount = "\"{}\": {} result{}";
            strings.relatedResultCount = "{} related result{}";
            strings.videoInfo = "Video by {}, [{}]";
            strings.playlistInfo = "Playlist by {}, [{} video{}]";
            strings.whatAreWePlaying = "What are we playing?";
            return SearchMessage(strings, &Cardinal, results);
        }

        /// @brief Create "Nothing is playing" message
        /// @return Ephemeral message
        virtual inline dpp::message nothingIsPlaying()
        {
            return ProblemMessage("Nothing is playing");
        }

        /// @brief Create "Paused <video>" or "Resumed <video>" message
        /// @param video Playing video
        /// @param paused Whether or not player is paused
        /// @return Normal message
        virtual inline dpp::message paused(const ytcpp::Video& video, bool paused)
        {
            return SuccessMessage(fmt::format(
                "{} *{}*",
                paused ? "Paused" : "Resumed",
                video.title()
            ));
        }
        
        /// @brief Create "Duration of video <video> is only <duration>!" message
        /// @param video Playing video
        /// @return Ephemeral message
        virtual inline dpp::message timestampOutOfBounds(const ytcpp::Video& video)
        {
            return ProblemMessage(fmt::format("Duration of video *{}* is `{}`", video.title(), Utility::NiceString(video.duration())));
        }

        /// @brief Create "Seeking <video> to <timestamp>" message
        /// @param video Seeking video
        /// @param timestamp Seek timestamp
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const ytcpp::Video& video, pt::time_duration timestamp, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format("Seeking *{}* to `{}`", video.title(), Utility::NiceString(timestamp)));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

/* Temporarily unsupported!
        /// @brief Create "Seeking <video> to chapter <chapter>, <timestamp>" message
        /// @param video Seeking video
        /// @param chapter The chapter in question
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const ytcpp::Video& video, const ytcpp::Video::Chapter& chapter, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format(
                "Seeking *{}* to chapter *{}: {}*, `{}`",
                video.title(),
                chapter.number,
                chapter.name,
                Utility::NiceString(chapter.timestamp)
            ));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "Video <video> has no chapters" message
        /// @param video The video in question
        /// @return Ephemeral message
        virtual inline dpp::message noChapters(const ytcpp::Video& video)
        {
            return ProblemMessage(fmt::format("Video *{}* has no chapters", video.title()));
        }

        /// @brief Get "Video <video> doesn't have such chapter" message
        /// @param video The video in question
        /// @return Ephemeral message
        virtual inline dpp::message unknownChapter(const ytcpp::Video& video)
        {
            return ProblemMessage(fmt::format("Video *{}* doesn't have such chapter", video.title()));
        }
*/
        

        /// @brief Create "Queue is empty" message
        /// @return Ephemeral message
        virtual inline dpp::message queueIsEmpty()
        {
            return ProblemMessage("Queue is empty");
        }

        /// @brief Create "There is only one item in queue" message
        /// @return Ephemeral message
        virtual inline dpp::message cantShuffle()
        {
            return ProblemMessage("There is only one item in queue");
        }

        /// @brief Create "Shuffled <count> items" message
        /// @param count Count of shuffled items
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message shuffled(size_t count, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format("Shuffled {} item{}", count, Cardinal(count)));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "Skipped item <item>" message
        /// @param item Skipped item
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message skipped(const ytcpp::Item& item, bool paused)
        {
            dpp::message message;
            switch (item.type())
            {
                case ytcpp::Item::Type::Video:
                    message = SuccessMessage(fmt::format("Skipped video *{}*", std::get<ytcpp::Video>(item).title()));
                    break;
                case ytcpp::Item::Type::Playlist:
                    message = SuccessMessage(fmt::format("Skipped playlist *{}*", std::get<ytcpp::Playlist>(item).title()));
                    break;
                default:
                    return unknownError();
            }

            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "No playlist is playing" message
        /// @return Ephemeral message
        virtual inline dpp::message noPlaylistIsPlaying()
        {
            return ProblemMessage("No playlist is playing");
        }

        /// @brief Create "Queue is cleared" message
        /// @return Normal message
        virtual inline dpp::message cleared()
        {
            return SuccessMessage("Queue is cleared");
        }

        /// @brief Create "Stopped playing and cleared queue" message
        /// @return Normal message
        virtual inline dpp::message stopped()
        {
            return SuccessMessage("Stopped playing and cleared queue");
        }

        /// @brief Add paused player warning to embed
        /// @param embed Embed to add paused player warning to
        virtual inline void pausedWarning(dpp::embed& embed)
        {
            embed.set_footer({ "Nothing is playing because player is paused! Use /pause to resume" });
        }

        /// @brief Create "Sorry, this button is no longer supported: use slashcommands instead" messagee
        /// @return Ephemeral message
        virtual inline dpp::message unsupportedButton()
        {
            return ProblemMessage("Sorry, this button is no longer supported: use slashcommands instead");
        }

        /// @brief Create "Something went wrong, I don't recognize this button" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownButton()
        {
            return ErrorMessage("Something went wrong, I don't recognize this button");
        }

        /// @brief Create "Something went wrong, I couldn't play <video>" message
        /// @param video The video in question
        /// @return Normal message
        virtual inline dpp::message playError(const ytcpp::Video& video)
        {
            return ErrorMessage(fmt::format("Something went wrong, couldn't play *{}*", video.title())).set_flags(0);
        }

        /// @brief Create "Something happened with my connection to Discord... Playing <video> from the start" message
        /// @param video The video in question
        /// @return Normal message
        virtual inline dpp::message reconnectedPlay(const ytcpp::Video& video)
        {
            return QuestionMessage(fmt::format(
                "Something happened with my connection to Discord... Playing *{}* from the start",
                video.title()
            ));
        }

        /// @brief Create session end message
        /// @param settings Guild's settings
        /// @param reason Session end reason
        /// @param session Player session
        /// @throw std::runtime_error if reason is unknown
        /// @return Normal message
        virtual inline dpp::message sessionEnd(const Settings& settings, EndReason reason, Session session)
        {
            EndStrings strings = {};
            strings.sessionInfo = "{}'s session #{} ended";
            strings.userRequested = "User asked me to leave";
            strings.timeout = "I was inactive";
            strings.timeoutCanBeChanged = "Timeout duration can\nbe changed with {}";
            strings.everybodyLeft = "Everybody left the voice channel";
            strings.kicked = "Somebody kicked me!";
            strings.voiceStatusNotCleared = {
                "Unfortunately, I was unable to clear voice channel status.\n"
                "Discord allows it to be modified only when I'm in the channel."
            };
            strings.moved = "Somebody moved me!";
            strings.sessionStats = "Session stats";
            strings.lasted = "Lasted";
            strings.tracksPlayed = "Tracks played";
            return EndMessage(strings, settings, reason, session);
        }

        /// @brief Create "Not playing" string
        /// @return "Not playing" string
        virtual inline const char* notPlaying()
        {
            return "Not playing";
        }

        /// @brief Create "[Paused]" string
        /// @return "[Paused]" string
        virtual inline const char* paused()
        {
            return "[Paused]";
        }

        /// @brief Create "Video" string
        /// @return "Video" string
        virtual inline const char* video()
        {
            return "Video";
        }

        /// @brief Create "Chapter" string
        /// @return "Chapter" string
        virtual inline const char* chapter()
        {
            return "Chapter";
        }
    };
}

} // namespace kb
