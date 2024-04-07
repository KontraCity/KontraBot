#pragma once

// STL modules
#include <string>
#include <deque>
#include <optional>
#include <mutex>
#include <thread>
#include <random>
#include <algorithm>

// Library DPP
#include <dpp/dpp.h>

// Library nlohmann::json
#include <nlohmann/json.hpp>

// Library boost::date_time
#include <boost/date_time.hpp>

// Library {fmt}
#include <fmt/format.h>

/* Forward Player class declaration for other modules */
namespace kc {
    namespace Bot {
        class Player;
    }
}

// Custom kc::Bot modules
#include "bot/bot.hpp"
#include "bot/info.hpp"
#include "bot/signal.hpp"
#include "bot/timeout.hpp"

// Custom kc::Youtube modules
#include "youtube/extractor.hpp"
#include "youtube/item.hpp"

namespace kc {

/* Namespace aliases and imports */
using nlohmann::json;
namespace dt = boost::gregorian;
namespace pt = boost::posix_time;

namespace Bot
{
    class Player
    {
    public:
        struct Session
        {
            struct EnqueuedItem
            {
                Youtube::Item item;
                dpp::user requester;
            };

            struct PlayingPlaylist
            {
                Youtube::Playlist playlist;
                Youtube::Playlist::Iterator iterator;
            };

            const dpp::snowflake guildId;
            const dpp::snowflake channelId;

            const uint64_t number;
            const dpp::user starter;
            pt::ptime startTimestamp;
            uint64_t tracksPlayed = 0;

            std::optional<Youtube::Video> playingVideo;
            std::optional<PlayingPlaylist> playingPlaylist;
            std::optional<dpp::user> playingRequester;
            std::deque<EnqueuedItem> queue;
            int64_t seekTimestamp = -1;
        };

    private:
        enum class ThreadStatus
        {
            Idle,
            Running,
            Stopped,
        };

    private:
        Bot* m_root;
        Timeout m_timeout;
        dpp::discord_client* m_client;
        Session m_session;

        std::mutex m_mutex;
        std::thread m_thread;
        ThreadStatus m_threadStatus = ThreadStatus::Idle;

    private:
        /// @brief Extract next video from queue or playlist iterator
        void extractNextVideo();

        /// @brief Increment count of played tracks
        void incrementPlayedTracks();

        /// @brief Start send thread if there is a playing video or enable timeout
        void checkPlayingVideo();

        /// @brief Send thread implementation
        void threadFunction();

        /// @brief Get current voice client
        /// @return Current voice client
        dpp::discord_voice_client* getVoiceClient();

        /// @brief Start send thread
        void startThread();

        /// @brief Stop send thread
        /// @param lock Acquired mutex lock
        void stopThread(std::unique_lock<std::mutex>& lock);

        /// @brief Timeout handler
        void timeoutHandler();

    public:
        /// @brief Initialize player
        /// @param root Player's bot
        /// @param client Discord client handling event
        /// @param interaction The event to handle
        /// @param info Guild's info
        Player(Bot* root, dpp::discord_client* client, const dpp::interaction& interaction, Info& info);

        ~Player();

        /// @brief Tell player that voice client is ready
        void signalReady();

        /// @brief Tell player that voice marker was passed
        void signalMarker();

        /// @brief Get player session
        /// @return Player session
        Session session();

        /// @brief Add item to player's queue
        /// @param item Item to add
        /// @param requester User that requested the item to be added
        void addItem(const Youtube::Item& item, const dpp::user& requester);

        /// @brief Check if player is paused
        /// @return True if player is paused
        bool paused();

        /// @brief Pause or resume audio playback
        bool pauseResume();

        /// @brief Seek playing video
        /// @param timestamp Timestamp to seek to
        void seek(uint64_t timestamp);

        /// @brief Shuffle items in queue
        void shuffle();

        /// @brief Skip playing video
        void skipVideo();

        /// @brief Skip playing playlist
        void skipPlaylist();

        /// @brief Clear queue
        void clear();

        /// @brief Stop audio playback and clear queue
        void stop();

        /// @brief End session message
        /// @param reason Session end reason reason
        void endSession(Locale::EndReason reason);
    };
}

} // namespace kc
