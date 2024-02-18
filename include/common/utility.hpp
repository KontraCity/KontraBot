#pragma once

// STL modules
#include <string>
#include <algorithm>

// Library Boost.Date_Time
#include <boost/date_time.hpp>

// Library {fmt}
#include <fmt/format.h>

namespace kc {

/* Namespace aliases and imports */
namespace dt = boost::gregorian;
namespace pt = boost::posix_time;

namespace Utility
{
    /// @brief Erase all commas characters in a string
    /// @param string String to modify
    void EraseCommas(std::string& string);

    /// @brief Convert date to string
    /// @param date Date to convert
    /// @return Converted date
    std::string ToString(dt::date date);

    /// @brief Convert duration to string
    /// @param duration Duration to convert
    /// @return Converted duration
    std::string ToString(pt::time_duration duration);

    /// @brief Convert number to string
    /// @param number Number to convert
    /// @return Converted number
    std::string ToString(uint64_t number);

    /// @brief Truncate string
    /// @param string String to truncate
    /// @param maxLength Max result string length
    /// @return Truncated string
    std::string Truncate(const std::string& string, size_t maxLength);
}

} // namespace kc