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

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

/* Forward Player class declaration for other modules */
namespace kc {
    namespace Bot {
        class Player;
    }
}

// Custom kc::Bot modules
#include "bot/info/info.hpp"
#include "bot/player/session.hpp"
#include "bot/bot.hpp"
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
    private:
        enum class ThreadStatus
        {
            Idle,
            Running,
            Stopped,
        };

    private:
        /// @brief Deduce frame's chapter from frame timestamp
        /// @param chapters Video chapters
        /// @param timestamp Frame's timestamp
        /// @return Frame's chapter
        static std::vector<Youtube::Video::Chapter>::const_iterator DeduceChapter(const std::vector<Youtube::Video::Chapter>& chapters, pt::time_duration timestamp);

    private:
        spdlog::logger m_logger;
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

        /// @brief Update playing chapter
        /// @param chapter The reached chapter
        void chapterReached(const Youtube::Video::Chapter& chapter);

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
        /// @param voiceChannelId ID of player's voice channel
        /// @param info Guild's info
        Player(Bot* root, dpp::discord_client* client, const dpp::interaction& interaction, dpp::snowflake voiceChannelId, Info& info);

        ~Player();

        /// @brief Tell player that voice client is ready
        void signalReady();

        /// @brief Tell player that voice marker was passed
        /// @param signalType Marker signal
        void signalMarker(const Signal& signal);

        /// @brief Update player's text channel
        /// @param channelId Text channel ID
        void updateTextChannel(dpp::snowflake channelId);

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
