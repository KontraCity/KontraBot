#pragma once

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale.hpp"

namespace kc {

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
            strings.prettyQuiet = "It's pretty quiet here";
            strings.nothingIsPlaying = "Nothing is playing. Go ahead and add something to queue!";
            strings.videoRequestedBy = "Video requested by {}:";
            strings.playlistRequestedBy = "Playlist requested by {}:";
            strings.videoInfo = {
                "by {}, [{}]\n"
                "{}, {} view{}"
            };
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
            strings.queueVideoInfo = {
                "Video by {}, [{}]\n"
                "Requested by {}"
            };
            strings.queuePlaylistInfo = {
                "Playlist by {}, [{} video{}]\n"
                "Requested by {}"
            };
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
            strings.hereAreTheStats = "Here are this guild's stats:";
            strings.interactionsProcessed = "Commands/buttons/options processed";
            strings.sessionsCount = "Sessions count";
            strings.tracksPlayed = "Tracks played";
            strings.timesKicked = "Times kicked";
            strings.timesMoved = "Times moved";
            return StatsMessage(strings, stats);
        }

        /// @brief Create "Sorry, I'm already taken!" message
        /// @return Ephemeral message
        virtual inline dpp::message alreadyTaken()
        {
            return ProblemMessage("Sorry, I'm already taken!");
        }
        
        /// @brief Randomly create "So be it" or "Whatever you want" message
        /// @return Normal message
        virtual inline dpp::message soBeIt()
        {
            switch (Utility::RandomNumber(0, 3))
            {
                default:
                    return SuccessMessage("So be it");
                case 1:
                    return SuccessMessage("Whatever you want");
                case 2:
                    return SuccessMessage("Alright");
                case 4:
                    return SuccessMessage("Carefully recorded");
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

        /// @brief Create youtube error message
        /// @param error Error in question
        /// @return Ephemeral message
        virtual inline dpp::message youtubeError(const Youtube::YoutubeError& error)
        {
            switch (error.type())
            {
                case Youtube::YoutubeError::Type::LoginRequired:
                    return ProblemMessage("This video is private");
                case Youtube::YoutubeError::Type::Unplayable:
                    return ProblemMessage("This video is blocked");
                case Youtube::YoutubeError::Type::YoutubeError:
                    return ProblemMessage("YouTube error occured");
                case Youtube::YoutubeError::Type::PlaylistError:
                    return ProblemMessage("This playlist is private");
                default:
                    return ProblemMessage("YouTube refused to let me play this item");
            }
        }

        /// @brief Create local error message
        /// @param error Error in question
        /// @return Ephemeral message
        virtual inline dpp::message localError(const Youtube::LocalError& error)
        {
            switch (error.type())
            {
                case Youtube::LocalError::Type::PlaylistItemsNotSupported:
                    return ProblemMessage("Sorry, I can't play any of the videos in the playlist");
                case Youtube::LocalError::Type::PlaylistNotSupported:
                    return ProblemMessage("Sorry, I can't play YouTube #Shorts playlists");
                case Youtube::LocalError::Type::EmptyPlaylist:
                    return ProblemMessage("This playlist is empty");
                default:
                    return unknownError();
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
        /// @throw std::invalid_argument if item is empty
        /// @return Normal message
        virtual inline dpp::message itemAdded(const Youtube::Item& item, bool paused, const std::optional<dpp::user>& requester = {})
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
        virtual inline dpp::message search(const Youtube::Results& results)
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
        virtual inline dpp::message paused(const Youtube::Video& video, bool paused)
        {
            return SuccessMessage(fmt::format(
                "{} **{}**",
                (paused ? "Paused" : "Resumed"),
                video.title()
            ));
        }
        
        /// @brief Create "Duration of video <video> is only <duration>!" message
        /// @param videoTitle Title of the video in question
        /// @param videoDuration Duration of the video in question
        /// @return Ephemeral message
        virtual inline dpp::message timestampOutOfBounds(const std::string& videoTitle, pt::time_duration videoDuration)
        {
            return ProblemMessage(fmt::format("Duration of video **{}** is only `{}`!", videoTitle, Utility::NiceString(videoDuration)));
        }

        /// @brief Create "Seeking <video> to <timestamp>" message
        /// @param videoTitle Seeking video title
        /// @param timestamp Seek timestamp
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const std::string& videoTitle, pt::time_duration timestamp, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format("Seeking **{}** to `{}`", videoTitle, Utility::NiceString(timestamp)));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "Seeking <video> to chapter <chapter>, <timestamp>" message
        /// @param videoTitle Seeking video title
        /// @param name Seek timestamp
        /// @param timestamp Timestamp string
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const std::string& videoTitle, const std::string& name, pt::time_duration timestamp, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format("Seeking **{}** to chapter **{}**, `{}`", videoTitle, name, Utility::NiceString(timestamp)));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "Video <video> has no chapters" message
        /// @param videoTitle Playing video title
        /// @return Ephemeral message
        virtual inline dpp::message noChapters(const std::string& videoTitle)
        {
            return ProblemMessage(fmt::format("Video **{}** has no chapters", videoTitle));
        }

        /// @brief Get "Video <video> doesn't have such chapter" message
        /// @param videoTitle Playing video title
        /// @return Ephemeral message
        virtual inline dpp::message unknownChapter(const std::string& videoTitle)
        {
            return ProblemMessage(fmt::format("Video **{}** doesn't have such chapter", videoTitle));
        }

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
        virtual inline dpp::message skipped(const Youtube::Item& item, bool paused)
        {
            dpp::message message;
            switch (item.type())
            {
                case Youtube::Item::Type::Video:
                    message = SuccessMessage(fmt::format("Skipped video **{}**", std::get<Youtube::Video>(item).title()));
                    break;
                case Youtube::Item::Type::Playlist:
                    message = SuccessMessage(fmt::format("Skipped playlist **{}**", std::get<Youtube::Playlist>(item).title()));
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

        /// @brief Create item autocomplete choice
        /// @param item Item to create autocomplete choice for
        /// @throw std::invalid_argument if item is empty
        /// @return Item autocomplete choice
        virtual inline dpp::command_option_choice itemAutocomplete(const Youtube::Item& item)
        {
            ItemAutocompleteStrings strings = {};
            strings.videoDescription = " - video by {}, [{}]";
            strings.livestreamDescription = " - livestream by {}";
            strings.premiereDescription = " - premiere by {}";
            strings.playlistDescription = " - playlist by {}, [{} video{}]";
            return ItemAutocompleteChoice(strings, &Cardinal, item);
        }

        /// @brief Create "Something went wrong, I don't recognize this button" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownButton()
        {
            return ErrorMessage("Something went wrong, I don't recognize this button");
        }

        /// @brief Create "Something went wrong, I don't recognize this option" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownOption()
        {
            return ErrorMessage(u8"Something went wrong, I don't recognize this option");
        }

        /// @brief Create "Skipping <livestream> because I can't play livestreams" message
        /// @param livestreamTitle Title of skipped livestream
        /// @return Normal message
        virtual inline dpp::message livestreamSkipped(const std::string& livestreamTitle)
        {
            return ProblemMessage(fmt::format("Skipping **{}** because I can't play livestreams", livestreamTitle));
        }

        /// @brief Create "Skipping <premiere> because I can't play premieres" message
        /// @param premiereTitle Title of skipped premiere
        /// @return Normal message
        virtual inline dpp::message premiereSkipped(const std::string& premiereTitle)
        {
            return ProblemMessage(fmt::format("Skipping **{}** because I can't play premieres", premiereTitle));
        }

        /// @brief Create "Something went wrong, I couldn't play <video>" message
        /// @param title Title of video in question
        /// @return Normal message
        virtual inline dpp::message playError(const std::string& videoTitle)
        {
            return ErrorMessage(fmt::format("Something went wrong, couldn't play **{}**", videoTitle)).set_flags(0);
        }

        /// @brief Create session end message
        /// @param reason Session end reason
        /// @param session Player session
        /// @return Normal message
        virtual inline dpp::message sessionEnd(EndReason reason, Session session)
        {
            EndStrings strings = {};
            strings.sessionInfo = "{}'s session #{} ended";
            strings.userRequested = "User asked me to leave";
            strings.timeout = "I was inactive";
            strings.everybodyLeft = "I was left alone in the voice channel";
            strings.kicked = "Somebody kicked me!";
            strings.moved = "Somebody moved me!";
            strings.lasted = "Lasted";
            strings.tracksPlayed = "Tracks played";
            return EndMessage(strings, reason, session);
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
    };
}

} // namespace kc
