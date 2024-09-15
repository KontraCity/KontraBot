#pragma once

// STL modules
#include <optional>
#include <string>
#include <random>
#include <chrono>
#include <thread>

// Library DPP
#include <dpp/dpp.h>

// Library Boost.Date_Time
#include <boost/date_time.hpp>

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

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
    /// @brief Create a logger
    /// @param name Logger name
    /// @param forceColor Whether to force sinks colors or not
    /// @return Created logger
    spdlog::logger CreateLogger(const std::string& name, std::optional<bool> forceColor = {});

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

    /// @brief Convert command to nice string
    /// @param command Command to convert
    /// @param option Command option if present
    /// @return Nice command string
    std::string NiceString(const dpp::slashcommand& command, const dpp::command_option& option = {});

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

    /// @brief Generate random number
    /// @param min Min number value
    /// @param max Max number value
    /// @return Generated number
    int64_t RandomNumber(int64_t min, int64_t max);

    /// @brief Perform execution delay
    /// @param seconds Amount of seconds to delay for
    void Sleep(double seconds);

    /// @brief Get Unix timestamp
    /// @return Unix timestamp
    int GetUnixTimestamp();
}

} // namespace kc
