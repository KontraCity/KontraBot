#include "common/utility.hpp"

namespace kc {

std::string Utility::ToString(dt::date date)
{
    return fmt::format(
        "{0} {1} {2}",
        date.day().as_number(),
        date.month().as_long_string(),
        static_cast<unsigned short>(date.year())
    );
}

std::string Utility::ToString(pt::time_duration duration)
{
    if (duration.hours())
        return fmt::format("{0}:{1:#02d}:{2:#02d}", duration.hours(), duration.minutes(), duration.seconds());
    return fmt::format("{0}:{1:#02d}", duration.minutes(), duration.seconds());
}

std::string Utility::ToString(uint64_t number)
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

    /*
    *   We need to be sure that truncated string is UTF-8 valid.
    *   Every UTF-8 sequence is at max 3 bytes long and starts with 11XX'XXXX byte.
    *   If this byte is encountered in last 3 bytes of string, the byte and following bytes must be erased.
    */
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
    return (result + "...");
}

} // namespace kc
