#include "bot/info.hpp"
using namespace kc::Bot::InfoConst;

namespace kc {

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
        throw std::runtime_error(fmt::format("kc::Bot::Info::Info(): Couldn't open info file [file: \"{}\"]", m_filePath));

    try
    {
        json infoJson = json::parse(infoFile);

        json settingsJson = infoJson[Fields::Settings];
        m_settings.locale = Locale::Create(settingsJson[Fields::Locale].get<std::string>());
        m_settings.timeoutMinutes = settingsJson[Fields::Timeout];
        m_settings.changeStatus = settingsJson[Fields::ChangeStatus];

        json statsJson = infoJson[Fields::Stats];
        m_stats.interactionsProcessed = statsJson[Fields::InteractionsProcessed];
        m_stats.sessionsCount = statsJson[Fields::SessionsCount];
        m_stats.tracksPlayed = statsJson[Fields::TracksPlayed];
        m_stats.timesKicked = statsJson[Fields::TimesKicked];
        m_stats.timesMoved = statsJson[Fields::TimesMoved];

        m_previousSettings = m_settings;
        m_previousStats = m_stats;
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format("kc::Bot::Info::Info(): Couldn't parse info JSON file [file: \"{}\", id: {}]", m_filePath, error.id));
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
    statsJson[Fields::SessionsCount] = m_stats.sessionsCount;
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

} // namespace kc
