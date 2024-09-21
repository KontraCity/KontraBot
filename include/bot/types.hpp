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
        uint64_t timeoutMinutes = 60;
        bool changeStatus = true;

        /// @brief Copy data from other settings
        /// @param other Other settings to copy data from
        /// @return Reference to these settings
        Settings& operator=(const Settings& other);

        /// @brief Check if settings are equal
        /// @param other Other settings to check against
        /// @return True if both settings are equal
        bool operator==(const Settings& other);
    };

    struct Stats
    {
        uint64_t interactionsProcessed = 0;
        uint64_t sessionsConducted = 0;
        uint64_t tracksPlayed = 0;
        uint64_t timesKicked = 0;
        uint64_t timesMoved = 0;

        /// @brief Add other stats
        /// @param other Other stats to add
        /// @return Reference to these stats
        Stats& operator+=(const Stats& other);

        /// @brief Check if stats are equal
        /// @param other Other stats to check against
        /// @return True if both stats are equal
        bool operator==(const Stats& other);
    };
}

} // namespace kc
