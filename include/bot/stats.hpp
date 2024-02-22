#pragma once

// STL modules
#include <string>
#include <fstream>
#include <filesystem>
#include <stdexcept>

// Library DPP
#include <dpp/dpp.h>

// Library nlohmann::json
#include <nlohmann/json.hpp>

// Library {fmt}
#include <fmt/format.h>

// Library spdlog
#include <spdlog/spdlog.h>

// Custom modules
#include "bot/locale/locale_en.hpp"
#include "bot/locale/locale_ru.hpp"

namespace kc {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Bot
{
    namespace StatsConst
    {
        // Directory where guild stats JSON files are stored
        constexpr const char* StatsDirectory = "stats";

        namespace Fields
        {
            namespace Locale
            {
                constexpr const char* Object = "locale";
                constexpr Bot::Locale::Type Default = LocaleEnConst::Type;
            }

            namespace TimeoutDuration
            {
                constexpr const char* Object = "timeout_duration";
                constexpr uint32_t Default = 60;
                constexpr uint32_t Max = 60 * 3;
            }
        }
    }

    class Stats
    {
    private:
        std::string m_filePath;
        Locale::Pointer m_locale;
        uint32_t m_timeoutDuration = 0;

    private:
        /// @brief Create locale from locale type
        /// @param localeType Locale type
        /// @return Created locale
        static Locale::Pointer CreateLocale(Locale::Type localeType);

        /// @brief Create locale from locale name
        /// @param localeName Locale name
        /// @return Created locale
        static Locale::Pointer CreateLocale(const std::string& localeName);

    private:
        /// @brief Save guild stats to file
        /// @throw std::runtime_error if internal error occurs
        void save();

    public:
        /// @brief Initialize guild stats
        /// @param guildId ID of guild
        /// @throw std::runtime_error if internal error occurs
        Stats(dpp::snowflake guildId);

        /// @brief Get guild's locale
        /// @return Guild's locale
        inline const Locale::Pointer& locale() const
        {
            return m_locale;
        }

        /// @brief Get guild's timeout duration
        /// @return Guild's timeout duration
        inline uint32_t timeoutDuration() const
        {
            return m_timeoutDuration;
        }

        /// @brief Change guild's locale
        /// @param locale Locale to change to
        void changeLocale(Locale::Type locale);

        /// @brief Change guild's timeout duration
        /// @param timeoutDuration Timeout duration to change to
        /// @throw std::invalid_argument if timeout duration is invalid
        void changeTimeoutDuration(int64_t timeoutDuration);
    };
}

} // namespace kc
