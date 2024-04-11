#include "bot/info/types.hpp"

namespace kc {

Bot::Settings& Bot::Settings::operator=(const Bot::Settings& other)
{
    if (*this == other)
        return *this;

    locale = Locale::Create(other.locale->type());
    timeoutMinutes = other.timeoutMinutes;
    return *this;
}

bool Bot::Settings::operator==(const Settings& other)
{
    if (!locale || !other.locale)
        return static_cast<bool>(locale) == static_cast<bool>(other.locale) && timeoutMinutes == other.timeoutMinutes;
    return locale->type() == other.locale->type() && timeoutMinutes == other.timeoutMinutes;
}

bool Bot::Stats::operator==(const Stats& other)
{
    return sessionsCount == other.sessionsCount && tracksPlayed == other.tracksPlayed;
}

} // namespace kc
