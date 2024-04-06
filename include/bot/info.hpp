#pragma once

// STL modules
#include <string>
#include <map>
#include <mutex>
#include <memory>
#include <filesystem>
#include <fstream>
#include <stdexcept>

// Library DPP
#include <dpp/dpp.h>

// Library nlohmann::json
#include <nlohmann/json.hpp>

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

/* Forward Settings and Stats structs declaration for other modules */
namespace kc {
    namespace Bot {
        struct Settings;
        struct Stats;
    }
}

// Custom modules
#include "bot/locale/locale_en.hpp"
#include "bot/locale/locale_ru.hpp"

namespace kc {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Bot
{
    struct Settings
    {
        Locale::Pointer locale;
        uint64_t timeoutMinutes;

        /// @brief Copy data from other settings
        /// @param other Othe settings to copy data from
        /// @return Reference to these settings
        Settings& operator=(const Settings& other);

        /// @brief Check if settings are equal
        /// @param other Other settings to check against
        /// @return True if both settings are equal
        bool operator==(const Settings& other);
    };

    struct Stats
    {
        uint64_t sessionsCount;
        uint64_t tracksPlayed;

        /// @brief Check if stats are equal
        /// @param other Other stats to check against
        /// @return True if both stats are equal
        bool operator==(const Stats& other);
    };

    namespace InfoConst
    {
        // Guilds info directory name
        constexpr const char* InfoDirectory = "info";

        namespace Fields
        {
            constexpr const char* Settings = "settings";
            constexpr const char* Locale = "locale";
            constexpr const char* Timeout = "timeout";

            constexpr const char* Stats = "stats";
            constexpr const char* SessionsCount = "sessions_count";
            constexpr const char* TracksPlayed = "tracks_played";
        }
    }

    class Info
    {
    public:
        /// @brief Create locale from its type
        /// @param localeType Locale type
        /// @return Created locale
        static Locale::Pointer CreateLocale(Locale::Type localeType);

        /// @brief Create locale from its name
        /// @param localeName Locale name
        /// @return Created locale
        static Locale::Pointer CreateLocale(const std::string localeName);

    private:
        /// @brief Get guild info file lock
        /// @param guildId ID of guild
        /// @return Guild info file lock
        static std::lock_guard<std::mutex> GetFileLock(dpp::snowflake guildId);

    private:
        static spdlog::logger Logger;

    private:
        std::lock_guard<std::mutex> m_fileLock;
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

} // namespace kc
