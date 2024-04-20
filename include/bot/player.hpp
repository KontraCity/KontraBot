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
#include "bot/bot.hpp"
#include "bot/info.hpp"
#include "bot/session.hpp"
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
        /// @param info Guild's info
        void extractNextVideo(const Info& info);

        /// @brief Increment count of played tracks
        /// @param info Guild's info
        void incrementPlayedTracks(Info& info);

        /// @brief Update playing chapter
        /// @param chapter The reached chapter
        /// @param info Guild's info
        void chapterReached(const Youtube::Video::Chapter& chapter, const Info& info);

        /// @brief Start send thread if there is a playing video or enable timeout
        void checkPlayingVideo();

        /// @brief Set voice channel status
        /// @param status Status to set
        void setStatus(std::string status);

        /// @brief Update voice channel status
        /// @param info Guild's info
        void updateStatus(const Info& info);

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

        /// @brief Signal bot to disconnect from voice channel
        /// @param reason Session end reason
        void signalDisconnect(Locale::EndReason reason);

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
        /// @param info Guild's info
        void signalReady(const Info& info);

        /// @brief Tell player that voice marker was passed
        /// @param signalType Marker signal
        /// @param info Guild's info
        void signalMarker(const Signal& signal, Info& info);

        /// @brief Update player's text channel
        /// @param channelId Text channel ID
        void updateTextChannel(dpp::snowflake channelId);

        /// @brief Update player's timeout
        /// @param info Guild's info
        void updateTimeout(const Info& info);

        /// @brief Update voice channel status
        /// @param info Guild's info
        void updateVoiceStatus(const Info& info);

        /// @brief Get player session
        /// @return Player session
        Session session();

        /// @brief Add item to player's queue
        /// @param item Item to add
        /// @param requester User that requested the item to be added
        /// @param info Guild's info
        void addItem(const Youtube::Item& item, const dpp::user& requester, const Info& info);

        /// @brief Check if player is paused
        /// @return True if player is paused
        bool paused();

        /// @brief Pause or resume audio playback
        /// @param info Guild's info
        bool pauseResume(const Info& info);

        /// @brief Seek playing video
        /// @param timestamp Timestamp to seek to
        /// @param info Guild's info
        void seek(uint64_t timestamp, const Info& info);

        /// @brief Shuffle items in queue
        void shuffle();

        /// @brief Skip playing video
        /// @param info Guild's info
        void skipVideo(Info& info);

        /// @brief Skip playing playlist
        /// @param info Guild's info
        void skipPlaylist(Info& info);

        /// @brief Clear queue
        void clear();

        /// @brief Stop audio playback and clear queue
        /// @param info Guild's info
        void stop(Info& info);

        /// @brief End player session
        /// @param info Guild's info
        /// @param dontClearVoiceStatus Whether or not voice status shouldn't be cleared
        void endSession(Info& info, bool dontClearVoiceStatus = false);

        /// @brief End player session
        /// @param info Guild's info
        /// @param reason Session end reason
        void endSession(Info& info, Locale::EndReason reason);
    };
}

} // namespace kc
