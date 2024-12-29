#include "bot/info.hpp"
using namespace kb::Bot::InfoConst;

// STL modules
#include <map>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <stdexcept>

// Library nlohmann::json
#include <nlohmann/json.hpp>

// Library Boost.Regex
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locales.hpp"
#include "common/utility.hpp"

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

Bot::Stats Bot::Info::GetGlobalStats()
{
    Stats globalStats = {};
    for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(InfoDirectory))
    {
        boost::smatch matches;
        std::string filename = file.path().filename().string();
        if (!boost::regex_search(filename, matches, boost::regex(R"(^(\d+)\.json$)")))
            continue;

        try
        {
            Info info(std::stoull(matches.str(1)));
            globalStats += info.stats();
        }
        catch (const std::runtime_error&)
        {
            /*
            *   Maybe it's not an info file?
            *   Why is it in info directory then?
            *   Ignore it.
            */
        }
    }
    return globalStats;
}

Bot::Info::Info(dpp::snowflake guildId)
    : m_logger(Utility::CreateLogger(fmt::format("info \"{}\"", static_cast<uint64_t>(guildId))))
    , m_filePath(fmt::format("{}/{}.json", InfoDirectory, static_cast<uint64_t>(guildId)))
{
    if (!std::filesystem::is_regular_file(m_filePath))
    {
        m_settings.locale = Locale::Create(LocaleEn::Type);
        m_previousSettings = m_settings;
        m_previousStats = m_stats;
        return;
    }

    std::ifstream infoFile(m_filePath);
    if (!infoFile)
        throw std::runtime_error(fmt::format("kb::Bot::Info::Info(): Couldn't open info file [file: \"{}\"]", m_filePath));

    try
    {
        json infoJson = json::parse(infoFile);

        json settingsJson = infoJson.at(Fields::Settings);
        m_settings.locale = Locale::Create(settingsJson.at(Fields::Locale).get<std::string>());
        m_settings.timeoutMinutes = settingsJson.at(Fields::Timeout);
        m_settings.changeStatus = settingsJson.at(Fields::ChangeStatus);

        json statsJson = infoJson.at(Fields::Stats);
        m_stats.interactionsProcessed = statsJson.at(Fields::InteractionsProcessed);
        m_stats.sessionsConducted = statsJson.at(Fields::SessionsConducted);
        m_stats.tracksPlayed = statsJson.at(Fields::TracksPlayed);
        m_stats.timesKicked = statsJson.at(Fields::TimesKicked);
        m_stats.timesMoved = statsJson.at(Fields::TimesMoved);

        m_previousSettings = m_settings;
        m_previousStats = m_stats;
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format("kb::Bot::Info::Info(): Couldn't parse info JSON file [file: \"{}\", id: {}]", m_filePath, error.id));
    }
}

Bot::Info::~Info()
{
    // There is no need to save anything if nothing was changed.
    if (m_settings == m_previousSettings && m_stats == m_previousStats)
        return;

    json settingsJson;
    settingsJson[Fields::Locale] = m_settings.locale->name();
    settingsJson[Fields::Timeout] = m_settings.timeoutMinutes;
    settingsJson[Fields::ChangeStatus] = m_settings.changeStatus;

    json statsJson;
    statsJson[Fields::InteractionsProcessed] = m_stats.interactionsProcessed;
    statsJson[Fields::SessionsConducted] = m_stats.sessionsConducted;
    statsJson[Fields::TracksPlayed] = m_stats.tracksPlayed;
    statsJson[Fields::TimesKicked] = m_stats.timesKicked;
    statsJson[Fields::TimesMoved] = m_stats.timesMoved;

    json infoJson;
    infoJson[Fields::Settings] = settingsJson;
    infoJson[Fields::Stats] = statsJson;

    std::ofstream infoFile(m_filePath, std::ios::trunc);
    if (!infoFile)
    {
        m_logger.error("Couldn't save info file");
        return;
    }
    infoFile << infoJson.dump(4) << '\n';
}

} // namespace kb
