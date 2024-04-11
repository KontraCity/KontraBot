#pragma once

// STL modules
#include <memory>

/* Forward Settings and Stats structs declaration for other modules */
namespace kc {
    namespace Bot {
        struct Settings;
        struct Stats;
    }
}

// Custom modules
#include "bot/locale/locale.hpp"

namespace kc {

namespace Bot
{
    struct Settings
    {
        std::unique_ptr<Locale> locale;
        uint64_t timeoutMinutes;
        bool changeStatus;

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
}

} // namespace kc
