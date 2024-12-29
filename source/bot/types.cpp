#include "bot/types.hpp"

namespace kb {

Bot::Settings::Settings(const Settings& other)
    : locale(Locale::Create(other.locale->type()))
    , timeoutMinutes(other.timeoutMinutes)
    , changeStatus(other.changeStatus)
{}

Bot::Settings& Bot::Settings::operator=(const Bot::Settings& other)
{
    if (*this == other)
        return *this;

    locale = Locale::Create(other.locale->type());
    timeoutMinutes = other.timeoutMinutes;
    changeStatus = other.changeStatus;
    return *this;
}

bool Bot::Settings::operator==(const Settings& other)
{
    if (!locale || !other.locale)
        return static_cast<bool>(locale) == static_cast<bool>(other.locale) && timeoutMinutes == other.timeoutMinutes && changeStatus == other.changeStatus;
    return locale->type() == other.locale->type() && timeoutMinutes == other.timeoutMinutes && changeStatus == other.changeStatus;
}

Bot::Stats& Bot::Stats::operator+=(const Stats& other)
{
    interactionsProcessed += other.interactionsProcessed;
    sessionsConducted += other.sessionsConducted;
    tracksPlayed += other.tracksPlayed;
    timesKicked += other.timesKicked;
    timesMoved += other.timesMoved;
    return *this;
}

bool Bot::Stats::operator==(const Stats& other)
{
    return interactionsProcessed == other.interactionsProcessed
        && sessionsConducted == other.sessionsConducted
        && tracksPlayed == other.tracksPlayed
        && timesKicked == other.timesKicked
        && timesMoved == other.timesMoved;
}

} // namespace kb
