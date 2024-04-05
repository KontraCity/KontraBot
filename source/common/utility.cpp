#include "common/utility.hpp"
#include <iostream>

namespace kc {

std::string Utility::NiceString(dt::date date)
{
    return fmt::format(
        "{} {} {}",
        date.day().as_number(),
        date.month().as_long_string(),
        static_cast<unsigned short>(date.year())
    );
}

std::string Utility::NiceString(pt::time_duration duration)
{
    if (duration.hours())
        return fmt::format("{}:{:#02d}:{:#02d}", duration.hours(), duration.minutes(), duration.seconds());
    return fmt::format("{}:{:#02d}", duration.minutes(), duration.seconds());
}

std::string Utility::NiceString(uint64_t number)
{
    std::string result = std::to_string(number);
    for (int index = static_cast<int>(result.size()) - 4; index >= 0; index -= 3)
        result.insert(index + 1, ",");
    return result;
}

std::string Utility::Truncate(const std::string& string, size_t maxLength)
{
    if (string.length() <= maxLength)
        return string;
    if (maxLength <= 3)
        return std::string(maxLength, '.');

    std::string result(string.begin(), string.begin() + maxLength - 3);
    for (int index = 0; index <= 3 && index < result.length(); ++index)
    {
        uint8_t byte = result[result.length() - index - 1];
        if ((byte & 0b1100'0000) == 0b1100'0000)
        {
            result.erase(result.end() - index - 1, result.end());
            break;
        }
    }
    return result + "...";
}

bool Utility::CaseInsensitiveStringContains(std::string string, std::string substring)
{
    utf8lwr(string.data());
    utf8lwr(substring.data());
    return static_cast<bool>(utf8str(string.data(), substring.data()));
}

} // namespace kc
