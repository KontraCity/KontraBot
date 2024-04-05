#pragma once

// STL modules
#include <string>
#include <random>

// Library Boost.Date_Time
#include <boost/date_time.hpp>

// Library {fmt}
#include <fmt/format.h>

// Library utf8.h
#include "external/utf8.h"

namespace kc {

/* Namespace aliases and imports */
namespace dt = boost::gregorian;
namespace pt = boost::posix_time;

namespace Utility
{
    /// @brief Convert date to nice string
    /// @param date Date to convert
    /// @return Nice date string
    std::string NiceString(dt::date date);

    /// @brief Convert duration to nice string
    /// @param duration Duration to convert
    /// @return Nice duration string
    std::string NiceString(pt::time_duration duration);

    /// @brief Convert number to nice string
    /// @param number Number to convert
    /// @return Nice number string
    std::string NiceString(uint64_t number);

    /// @brief Truncate string to limited length
    /// @param string String to truncate
    /// @param maxLength Max string length
    /// @return Truncated string
    std::string Truncate(const std::string& string, size_t maxLength);

    /// @brief Case insensitively check if UTF-8 string contains substring
    /// @param string String to search in
    /// @param substring String to search for
    /// @return True if string contains substring
    bool CaseInsensitiveStringContains(std::string string, std::string substring);
}

} // namespace kc
