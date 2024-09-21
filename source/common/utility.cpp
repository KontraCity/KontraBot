#include "common/utility.hpp"

namespace kc {

spdlog::logger Utility::CreateLogger(const std::string& name, std::optional<bool> forceColor)
{
    static auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    if (forceColor)
    {
        /*
        *   This function is initialized when [forceColor] parameter is passed.
        *   This is why the sink is configured here.
        */
        sink->set_pattern("[%^%d.%m.%C %H:%M:%S %L%$] [%n] %v");
        if (*forceColor)
            sink->set_color_mode(spdlog::color_mode::always);
    }
    return { name, {sink} };
}

std::string Utility::NiceString(dt::date date)
{
    return fmt::format(
        "{:#02d}.{:#02d}.{:#04d}",
        date.day().as_number(),
        date.month().as_number(),
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

std::string Utility::NiceString(const dpp::slashcommand& command, const dpp::command_option& option)
{
    if (option.name.empty())
        return fmt::format("</{}:{}>", command.name, static_cast<uint64_t>(command.id));
    return fmt::format("</{} {}:{}>", command.name, option.name, static_cast<uint64_t>(command.id));
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

int64_t Utility::RandomNumber(int64_t min, int64_t max)
{
    static std::random_device randomDevice;
    static std::mt19937_64 generator(randomDevice());
    return std::uniform_int_distribution(min, max)(generator);
}

void Utility::Sleep(double seconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<size_t>(seconds * 1'000'000)));
}

int Utility::GetUnixTimestamp()
{
    constexpr pt::ptime epoch(dt::date(1970, 1, 1));
    pt::ptime now = pt::second_clock::local_time();
    return static_cast<int>((now - epoch).total_seconds());
}

pt::time_duration Utility::TimeToNextMinute()
{
    pt::ptime now = pt::microsec_clock::local_time();
    pt::ptime nextMinute = now + pt::minutes(1);
    return pt::ptime(nextMinute.date(), pt::time_duration(nextMinute.time_of_day().hours(), nextMinute.time_of_day().minutes(), 0)) - now;
}

} // namespace kc
