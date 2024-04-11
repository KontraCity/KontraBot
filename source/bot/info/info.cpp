#include "bot/info/info.hpp"
using namespace kc::Bot::InfoConst;

namespace kc {

std::lock_guard<std::mutex> Bot::Info::GetFileLock(dpp::snowflake guildId)
{
    static std::mutex mutex;
    std::lock_guard lock(mutex);

    static std::map<dpp::snowflake, std::mutex> fileMutexes;
    return std::lock_guard(fileMutexes[guildId]);
}

spdlog::logger Bot::Info::Logger("info", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

Bot::Info::Info(dpp::snowflake guildId)
    : m_fileLock(GetFileLock(guildId))
    , m_filePath(fmt::format("{}/{}.json", InfoDirectory, static_cast<uint64_t>(guildId)))
{
    if (!std::filesystem::is_regular_file(m_filePath))
    {
        m_settings.locale = std::make_unique<LocaleEn>();
        m_settings.timeoutMinutes = 60;
        m_settings.changeStatus = true;

        m_stats.sessionsCount = 0;
        m_stats.tracksPlayed = 0;

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
        m_stats.sessionsCount = statsJson[Fields::SessionsCount];
        m_stats.tracksPlayed = statsJson[Fields::TracksPlayed];

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
    statsJson[Fields::SessionsCount] = m_stats.sessionsCount;
    statsJson[Fields::TracksPlayed] = m_stats.tracksPlayed;

    json infoJson;
    infoJson[Fields::Settings] = settingsJson;
    infoJson[Fields::Stats] = statsJson;

    std::ofstream infoFile(m_filePath, std::ios::trunc);
    if (!infoFile)
    {
        Logger.error("Couldn't save info file \"{}\"", m_filePath);
        return;
    }
    infoFile << infoJson.dump(4) << '\n';
}

} // namespace kc
