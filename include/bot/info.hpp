#pragma once

// STL modules
#include <string>

// Library DPP
#include <dpp/dpp.h>

// Library spdlog
#include <spdlog/spdlog.h>

// Custom modules
#include "bot/types.hpp"

namespace kb {

namespace Bot
{
    namespace InfoConst
    {
        // Guilds info directory name
        constexpr const char* InfoDirectory = "info";

        namespace Fields
        {
            constexpr const char* Settings = "settings";
            constexpr const char* Locale = "locale";
            constexpr const char* Timeout = "timeout";
            constexpr const char* ChangeStatus = "change_status";

            constexpr const char* Stats = "stats";
            constexpr const char* InteractionsProcessed = "interactions_processed";
            constexpr const char* SessionsConducted = "sessions_conducted";
            constexpr const char* TracksPlayed = "tracks_played";
            constexpr const char* TimesKicked = "times_kicked";
            constexpr const char* TimesMoved = "times_moved";
        }
    }

    class Info
    {
    public:
        /// @brief Get global stats
        /// @return Global stats
        static Stats GetGlobalStats();

    private:
        spdlog::logger m_logger;
        std::string m_filePath;
        Settings m_previousSettings;
        Settings m_settings;
        Stats m_previousStats;
        Stats m_stats;

    public:
        /// @brief Initialize guild info
        /// @param guildId ID of guild
        /// @throw std::runtime_error if internal error occurs
        Info(dpp::snowflake guildId);

        ~Info();

    public:
        /// @brief Get guild settings
        /// @return Guild settings
        inline const Settings& settings() const
        {
            return m_settings;
        }

        /// @brief Get guild settings
        /// @return Guild settings
        inline Settings& settings()
        {
            return m_settings;
        }

        /// @brief Get guild stats
        /// @return Guild stats
        inline const Stats& stats() const
        {
            return m_stats;
        }

        /// @brief Get guild stats
        /// @return Guild stats
        inline Stats& stats()
        {
            return m_stats;
        }
    };
}

} // namespace kb
