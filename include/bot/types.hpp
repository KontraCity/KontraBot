#pragma once

// STL modules
#include <memory>

/* Forward kb::Bot::Settings and kb::Bot::Stats structs declaration for other modules */
namespace kb {
    namespace Bot {
        struct Settings;
        struct Stats;
    }
}

// Custom modules
#include "bot/locale/locale.hpp"

namespace kb {

namespace Bot
{
    struct Settings
    {
        std::unique_ptr<Locale> locale;
        uint64_t timeoutMinutes = 60;
        bool changeStatus = true;

        Settings() = default;

        Settings(const Settings& other);

        /// @brief Copy data from other settings
        /// @param other Other settings to copy data from
        /// @return Reference to these settings
        Settings& operator=(const Settings& other);

        /// @brief Check if settings are equal
        /// @param other Other settings to check against
        /// @return True if both settings are equal
        bool operator==(const Settings& other) const;
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
        bool operator==(const Stats& other) const;
    };
}

} // namespace kb
