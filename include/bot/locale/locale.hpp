#pragma once

// STL modules
#include <memory>
#include <string>
#include <optional>

// Library DPP
#include <dpp/snowflake.h>

/* Forward kc::Bot::Locale class declaration for other modules */
namespace kc {
    namespace Bot {
        class Locale;
    }
}

// Custom kc::Bot modules
#include "bot/session.hpp"
#include "bot/types.hpp"

// Custom kc::Youtube modules
#include "youtube/error.hpp"
#include "youtube/item.hpp"
#include "youtube/search.hpp"

namespace kc {

namespace Bot
{
    namespace LocaleConst
    {
        constexpr size_t MaxSessionItemsShown = 5;

        namespace Colors
        {
            constexpr uint32_t Success = dpp::colors::dark_green;
            constexpr uint32_t Question = dpp::colors::yellow;
            constexpr uint32_t Problem = dpp::colors::red;
            constexpr uint32_t Error = dpp::colors::dark_red;
            constexpr uint32_t End = 0x0ea8ea;
            constexpr uint32_t BadEnd = dpp::colors::yellow;
        }

        namespace Emojis
        {
            constexpr const char* Success = u8"✅";
            constexpr const char* Question = u8"❔";
            constexpr const char* Problem = u8"❌";
            constexpr const char* Error = u8"💀";
            constexpr const char* Boring = u8"🥱";
            constexpr const char* Play = u8"▶️";
            constexpr const char* Playlist = u8"🔢";
            constexpr const char* Search = u8"🔍";
        }
    }

    class Locale
    {
    public:
        // Locale instance pointer
        using Pointer = std::unique_ptr<Locale>;

        enum class Type
        {
            English,
            Russian,
        };

        enum class EndReason
        {
            UserRequested,  // User issued /leave command
            Timeout,        // Inactivity timeout happened
            EverybodyLeft,  // Everybody left voice channel
            Kicked,         // Somebody kicked bot from voice channel
            Moved,          // Somebody moved bot in another voice channel
        };

    public:
        /// @brief Create locale from its type
        /// @param localeType Locale type
        /// @return Created locale
        static Pointer Create(Type localeType);

        /// @brief Create locale from its name
        /// @param localeName Locale name
        /// @return Created locale
        static Pointer Create(const std::string localeName);

        /// @brief Convert end reason to string
        /// @param reason The end reason to convert
        /// @return Converted string
        static const char* EndReasonToString(EndReason reason);

    protected:
        // Function used to get number's cardinal ending
        using CardinalFunction = const char* (*)(size_t);

        struct MentionReplyStrings
        {
            const char* ifYouNeedAnyHelp;       // "If you need any help about how I play music, use {}" string
            const char* differentLanguagesHint; // "I can speak different languages!" <newline> "Now its <language>" <newline> "Use {} if you need to change it" string in other language
        };

        struct HelpStrings
        {
            const char* hereAreAllOfMyCommands; // "Here are all of my commands" string
            const char* differentLanguagesHint; // "I can speak different languages!" <newline> "Now its <language>" <newline> "Use {} if you need to change it" string in other language
            const char* helpDescription;        // /help command description
            const char* sessionDescription;     // /session command description
            const char* sessionFaq;             // /session command FAQ
            const char* settingsDescription;    // /settings command description
            const char* statsDescription;       // /stats command description
            const char* setDescription;         // /set language, /set timeout, /set change-status commands description
            const char* setFaq;                 // /set command FAQ
            const char* joinDescription;        // /join command description
            const char* leaveDescription;       // /leave command description
            const char* playDescription;        // /play command description
            const char* playFaq;                // /play command FAQ
            const char* pauseDescription;       // /pause command description
            const char* seekDescription;        // /seek command description
            const char* seekFaq;                // /seek command FAQ
            const char* shuffleDescription;     // /shuffle command description
            const char* skipDescription;        // /skip command description
            const char* clearDescription;       // /clear command description
            const char* stopDescription;        // /stop command description
        };

        struct SessionStrings
        {
            const char* infoFooter;             // "Session lasted [{}] | Tracks played: [{}]" string
            const char* prettyQuiet;            // "It's pretty quiet here" string
            const char* nothingIsPlaying;       // "Nothing is playing. Go ahead and add something to queue!" string
            const char* video;                  // "Video" string
            const char* chapter;                // "Chapter" string
            const char* videoInfo;              // "by {}, [{}]" <newline> "{}, {} view{}" string
            const char* requestedBy;            // "Requested by {}" string
            const char* lastVideoPlaylistInfo;  // "Playlist by {}" <newline> "Last video is playing" string
            const char* playlistInfo;           // "Playlist by {}" <newline> "{} video{} left" string
            const char* morePlaylistVideos;     // "... {} more video{}" string
            const char* playlistVideoInfo;      // "Video by {}, [{}]" string
            const char* playlistLivestreamInfo; // "Livestream by {}" <newline> "Will be skipped because I can't play them!" string
            const char* playlistPremiereInfo;   // "Premiere by {}" <newline> "Will be skipped because I can't play them!" string
            const char* queueIsEmpty;           // "Queue is empty" string
            const char* queueInfo;              // "Queue: {} item{}" string
            const char* moreQueueItems;         // "... {} more item{}" string
            const char* queueVideoInfo;         // "Video by {}, [{}]" string
            const char* queuePlaylistInfo;      // "Playlist by {}, [{} video{}]" string
        };

        struct SettingsStrings
        {
            const char* hereAreTheSettings;     // "Here are this guild's settings" string
            const char* language;               // "Language" string
            const char* timeoutDuration;        // "Timeout duration" string
            const char* changeStatus;           // "Allowed to change voice channel status" string
            const char* yes;                    // "Yes" string
            const char* no;                     // "No" string
        };

        struct StatsStrings
        {
            const char* hereAreTheStats;        // "Here are this guild's stats:" string
            const char* interactionsProcessed;  // "Commands/buttons/options processed" string
            const char* sessionsConducted;      // "Sessions conducted" string
            const char* tracksPlayed;           // "Tracks played" string
            const char* timesKicked;            // "Times kicked" string
            const char* timesMoved;             // "Times moved" string
        };

        struct AmbigousPlayStrings
        {
            const char* playPlaylistOrVideo;    // "Play whole playlist or just this video?" string
            const char* playVideo;              // "Play video" string
            const char* playPlaylist;           // "Play playlist" string
        };

        struct ItemAddedStrings
        {
            const char* requestedBy;            // "Requested by {}" string
            const char* videoAdded;             // "Video added to queue" string
            const char* playlistAdded;          // "Playlist added to queue" string
            const char* playlistInfo;           // "{} [{} video{}]" string
            const char* playAgain;              // "Play again" string
            const char* related;                // "Related" string
            const char* youtube;                // "YouTube" string
        };

        struct SearchStrings
        {
            const char* noResults;              // "No results" string
            const char* resultCount;            // ""{}": {} result{}" string
            const char* relatedResultCount;     // "{} related result{}" string
            const char* videoInfo;              // "Video by {}, [{}]" string
            const char* playlistInfo;           // "Playlist by {}, [{} video{}]" string
            const char* whatAreWePlaying;       // "What are we playing?" string
        };

        struct EndStrings
        {
            const char* sessionInfo;            // "{}'s session #{} ended" string
            const char* userRequested;          // "User asked me to leave" string
            const char* timeout;                // "I was inactive" string
            const char* timeoutCanBeChanged;    // "Timeout duration can be changed with {}" string
            const char* everybodyLeft;          // "Everybody left voice channel" string
            const char* kicked;                 // "Somebody kicked me!" string
            const char* voiceStatusNotCleared;  // "Unfortunately, I was unable to clear voice status." <newline> "Discord only allows it to be modified when I'm in the voice channel." string
            const char* moved;                  // "Somebody moved me!" string
            const char* sessionStats;           // "Session stats" string
            const char* lasted;                 // "Lasted" string
            const char* tracksPlayed;           // "Tracks played" string
        };

    protected:
        /// @brief Create generic message
        /// @param color Embed color
        /// @param emoji Embed emoji
        /// @param string Message string
        /// @param ephemeral Whether the message should be ephemeral or not
        /// @return Generic message
        static dpp::message GenericMessage(uint32_t color, const char* emoji, const std::string& string, bool ephemeral);

        /// @brief Create success message
        /// @param string Message string
        /// @return Normal message
        static dpp::message SuccessMessage(const std::string& string);

        /// @brief Create question message
        /// @param string Message string
        /// @return Ephemeral message
        static dpp::message QuestionMessage(const std::string& string);

        /// @brief Create problem message
        /// @param string Message string
        /// @return Ephemeral message
        static dpp::message ProblemMessage(const std::string& string);

        /// @brief Create error message
        /// @param string Message string
        /// @return Ephemeral message
        static dpp::message ErrorMessage(const std::string& string);

        /// @brief Create mention reply message
        /// @param strings Locale's mention reply strings
        /// @return Normal message
        static dpp::message MentionReplyMessage(const MentionReplyStrings& strings);

        /// @brief Create help message
        /// @param strings Locale's help strings
        /// @return Ephemeral message
        static dpp::message HelpMessage(const HelpStrings& strings);

        /// @brief Create session message
        /// @param strings Locale's session message
        /// @param cardinalFunction Locale's cardinal ending function
        /// @param session Player session
        /// @return Ephemeral message
        static dpp::message SessionMessage(const SessionStrings& strings, CardinalFunction cardinalFunction, const Session& session);

        /// @brief Create guild settings message
        /// @param strings Locale's settings strings
        /// @param settings Guild settings
        /// @return Normal message
        static dpp::message SettingsMessage(const SettingsStrings& strings, const Settings& settings);

        /// @brief Create guild stats message
        /// @param strings Locale's stats strings
        /// @param stats Guild stats
        /// @return Normal message
        static dpp::message StatsMessage(const StatsStrings& strings, const Stats& stats);

        /// @brief Create ambiguous play message
        /// @param strings Locale's ambigous play strings
        /// @param videoId ID of video to play
        /// @param playlistId ID of playlist to play
        /// @return Ephemeral message
        static dpp::message AmbiguousPlayMessage(const AmbigousPlayStrings& strings, const std::string& videoId, const std::string& playlistId);

        /// @brief Create "Item added to queue" message
        /// @param strings Locale's item added strings
        /// @param item Added item
        /// @param cardinalFunction Locale's cardinal ending function
        /// @param requester User that added the item if needs to be shown in message
        /// @throw std::runtime_error if item type is unknown
        /// @return Normal message
        static dpp::message ItemAddedMessage(const ItemAddedStrings& strings, const Youtube::Item& item, CardinalFunction cardinalFunction, const std::optional<dpp::user>& requester = {});

        /// @brief Create search message
        /// @param strings Locale's search strings
        /// @param cardinalFunction Locale's cardinal ending function
        /// @param results Search results
        /// @return Ephemeral message
        static dpp::message SearchMessage(const SearchStrings& strings, CardinalFunction cardinalFunction, const Youtube::Results& results);

        /// @brief Create session end message
        /// @param strings Locale's session end strings
        /// @param settings Guild's settings
        /// @param reason Session end reason
        /// @param session Player session
        /// @throw std::runtime_error if reason is unknown
        /// @return Normal message
        static dpp::message EndMessage(const EndStrings& strings, const Settings& settings, EndReason reason, Session session);

    public:
        /// @brief Get locale type
        /// @return Locale type
        virtual inline Type type() = 0;

        /// @brief Get locale name
        /// @return Locale name
        virtual inline const char* name() = 0;

        /// @brief Get long locale name
        /// @return Long locale name
        virtual inline const char* longName() = 0;

        /// @brief Create mention reply message
        /// @return Mention reply message
        virtual inline dpp::message mention() = 0;

        /// @brief Create help message
        /// @return Ephemeral message
        virtual inline dpp::message help() = 0;

        /// @brief Create "I am not sitting in any voice channel" message
        /// @return Ephemeral message
        virtual inline dpp::message botNotInVoiceChannel() = 0;

        /// @brief Create session message
        /// @param session Player session
        /// @return Ephemeral message
        virtual inline dpp::message session(const Session& session) = 0;

        /// @brief Create guild settings message
        /// @param settings Guild settings
        /// @return Normal message
        virtual inline dpp::message settings(const Settings& settings) = 0;

        /// @brief Create guild stats message
        /// @param stats Guild stats
        /// @return Normal message
        virtual inline dpp::message stats(const Stats& stats) = 0;

        /// @brief Create "Only users sitting with me can control the player" message
        /// @return Ephemeral message
        virtual inline dpp::message onlyUsersWithMeCanControlPlayer() = 0;

        /// @brief Randomly create "So be it" or "Whatever you want" message
        /// @return Normal message
        virtual inline dpp::message soBeIt() = 0;

        /// @brief Create "Inactivity timeout duration must be greater than zero" message
        /// @return Ephemeral message
        virtual inline dpp::message badTimeoutDuration() = 0;

        /// @brief Create "Sorry, I'm not ready to sit and do nothing for more than two hours" message
        /// @return Ephemeral message
        virtual inline dpp::message longTimeoutDuration() = 0;

        /// @brief Create "Inactivity timeout duration is set to <duration>" message
        /// @param timeoutMinutes Duration of timeout in minutes
        /// @return Ephemeral message
        virtual inline dpp::message timeoutDurationSet(uint64_t timeoutMinutes) = 0;

        /// @brief Create "Something went wrong, I don't recognize this command" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownCommand() = 0;

        /// @brief Create "Joining <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Normal message
        virtual inline dpp::message joining(dpp::snowflake channelId) = 0;

        /// @brief Create "Already joined <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Ephemeral message
        virtual inline dpp::message alreadyJoined(dpp::snowflake channelId) = 0;

        /// @brief Create "I'm already sitting in another voice channel" message
        /// @param channelId ID of voice channel in question
        /// @return Ephemeral message
        virtual inline dpp::message cantJoin() = 0;

        /// @brief Create "Join a voice channel first" message
        /// @return Ephemeral message
        virtual inline dpp::message userNotInVoiceChannel() = 0;

        /// @brief Crate "I can't play in AFK channels!" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayInAfkChannels() = 0;

        /// @brief Create "Left <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Normal message
        virtual inline dpp::message left(dpp::snowflake channelId) = 0;

        /// @brief Create ambiguous play message
        /// @param videoId ID of video to play
        /// @param playlistId ID of playlist to play
        /// @return Ephemeral message
        virtual inline dpp::message ambiguousPlay(const std::string& videoId, const std::string& playlistId) = 0;

        /// @brief Create "Sorry, I can't play livestreams" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayLivestreams() = 0;

        /// @brief Create "Sorry, I can't play premieres" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayPremieres() = 0;

        /// @brief Create youtube error message
        /// @param error Error in question
        /// @return Ephemeral message
        virtual inline dpp::message youtubeError(const Youtube::YoutubeError& error) = 0;

        /// @brief Create local error message
        /// @param error Error in question
        /// @return Ephemeral message
        virtual inline dpp::message localError(const Youtube::LocalError& error) = 0;

        /// @brief Create unknown error message
        /// @return Ephemeral message
        virtual inline dpp::message unknownError() = 0;

        /// @brief Create "Item added to queue" message
        /// @param item Added item
        /// @param paused Whether or not to display paused player warning
        /// @param requester User that added the item if needs to be shown in message
        /// @throw std::runtime_error if item type is unknown
        /// @return Normal message
        virtual inline dpp::message itemAdded(const Youtube::Item& item, bool paused, const std::optional<dpp::user>& requester = {}) = 0;

        /// @brief Create search message
        /// @param results Search results
        /// @return Ephemeral message
        virtual inline dpp::message search(const Youtube::Results& results) = 0;

        /// @brief Create "Nothing is playing" message
        /// @return Ephemeral message
        virtual inline dpp::message nothingIsPlaying() = 0;

        /// @brief Create "Paused <video>" or "Resumed <video>" message
        /// @param video Playing video
        /// @param paused Whether or not player is paused
        /// @return Normal message
        virtual inline dpp::message paused(const Youtube::Video& video, bool paused) = 0;

        /// @brief Create "Duration of video <video> is only <duration>!" message
        /// @param video Playing video
        /// @return Ephemeral message
        virtual inline dpp::message timestampOutOfBounds(const Youtube::Video& video) = 0;

        /// @brief Create "Seeking <video> to <timestamp>" message
        /// @param video Seeking video
        /// @param timestamp Seek timestamp
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const Youtube::Video& video, pt::time_duration timestamp, bool paused) = 0;

        /// @brief Create "Seeking <video> to chapter <chapter>, <timestamp>" message
        /// @param video Seeking video
        /// @param chapter The chapter in question
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const Youtube::Video& video, const Youtube::Video::Chapter& chapter, bool paused) = 0;

        /// @brief Create "Video <video> has no chapters" message
        /// @param video The video in question
        /// @return Ephemeral message
        virtual inline dpp::message noChapters(const Youtube::Video& video) = 0;

        /// @brief Get "Video <video> doesn't have such chapter" message
        /// @param video The video in question
        /// @return Ephemeral message
        virtual inline dpp::message unknownChapter(const Youtube::Video& video) = 0;

        /// @brief Create "Queue is empty" message
        /// @return Ephemeral message
        virtual inline dpp::message queueIsEmpty() = 0;

        /// @brief Create "There is only one item in queue" message
        /// @return Ephemeral message
        virtual inline dpp::message cantShuffle() = 0;

        /// @brief Create "Shuffled <count> items" message
        /// @param count Count of shuffled items
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message shuffled(size_t count, bool paused) = 0;

        /// @brief Create "Skipped item <item>" message
        /// @param item Skipped item
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message skipped(const Youtube::Item& item, bool paused) = 0;

        /// @brief Create "No playlist is playing" message
        /// @return Ephemeral message
        virtual inline dpp::message noPlaylistIsPlaying() = 0;

        /// @brief Create "Queue is cleared" message
        /// @return Normal message
        virtual inline dpp::message cleared() = 0;

        /// @brief Create "Stopped playing and cleared queue" message
        /// @return Normal message
        virtual inline dpp::message stopped() = 0;

        /// @brief Add paused player warning to embed
        /// @param embed Embed to add paused player warning to
        virtual inline void pausedWarning(dpp::embed& embed) = 0;

        /// @brief Create "Sorry, this button is no longer supported: use slashcommands instead" messagee
        /// @return Ephemeral message
        virtual inline dpp::message unsupportedButton() = 0;

        /// @brief Create "Something went wrong, I don't recognize this button" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownButton() = 0;

        /// @brief Create "Something went wrong, I couldn't play <video>" message
        /// @param video The video in question
        /// @return Normal message
        virtual inline dpp::message playError(const Youtube::Video& video) = 0;

        /// @brief Create "Something happened with my connection to Discord... Playing <video> from the start" message
        /// @param video The video in question
        /// @return Normal message
        virtual inline dpp::message reconnectedPlay(const Youtube::Video& video) = 0;

        /// @brief Create session end message
        /// @param settings Guild's settings
        /// @param reason Session end reason
        /// @param session Player session
        /// @throw std::runtime_error if reason is unknown
        /// @return Normal message
        virtual inline dpp::message sessionEnd(const Settings& settings, EndReason reason, Session session) = 0;

        /// @brief Create "Not playing" string
        /// @return "Not playing" string
        virtual inline const char* notPlaying() = 0;

        /// @brief Create "[Paused]" string
        /// @return "[Paused]" string
        virtual inline const char* paused() = 0;

        /// @brief Create "Video" string
        /// @return "Video" string
        virtual inline const char* video() = 0;

        /// @brief Create "Chapter" string
        /// @return "Chapter" string
        virtual inline const char* chapter() = 0;
    };
}

} // namespace kc
