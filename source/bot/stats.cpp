#include "bot/stats.hpp"

namespace kc {

std::unique_ptr<Bot::Locale> Bot::Stats::CreateLocale(Locale::Type localeType)
{
    switch (localeType)
    {
        default:
            return std::make_unique<LocaleEn>();
        case LocaleRuConst::Type:
            return std::make_unique<LocaleRu>();
    }
}

std::unique_ptr<Bot::Locale> Bot::Stats::CreateLocale(const std::string& localeName)
{
    if (localeName == LocaleRuConst::Name)
        return CreateLocale(LocaleRuConst::Type);
    return CreateLocale(LocaleEnConst::Type);
}

void Bot::Stats::save()
{
    json statsJson;
    statsJson[StatsConst::Fields::Locale::Object] = m_locale->LocaleName();
    statsJson[StatsConst::Fields::TimeoutDuration::Object] = m_timeoutDuration;

    std::ofstream statsFile(m_filePath, std::ios::trunc);
    if (!statsFile)
    {
        throw std::runtime_error(fmt::format(
            "kc::Bot::Stats::save(): "
            "Couldn't open stats file [file: \"{}\"]",
            m_filePath
        ));
    }
    statsFile << statsJson.dump(4) << '\n';
}

Bot::Stats::Stats(dpp::snowflake guildId)
    : m_filePath(fmt::format("{}/{}.json", StatsConst::StatsDirectory, static_cast<uint64_t>(guildId)))
{
    if (!std::filesystem::is_regular_file(m_filePath))
    {
        m_locale = CreateLocale(StatsConst::Fields::Locale::Default);
        m_timeoutDuration = StatsConst::Fields::TimeoutDuration::Default;

        save();
        return;
    }

    std::ifstream statsFile(m_filePath);
    if (!statsFile)
    {
        throw std::runtime_error(fmt::format(
            "kc::Bot::Stats::Stats(): "
            "Couldn't open stats file [file: \"{}\"]",
            m_filePath
        ));
    }

    try
    {
        json statsJson = json::parse(statsFile);
        bool saveNeeded = false;

        std::string localeString = statsJson[StatsConst::Fields::Locale::Object];
        if (localeString != LocaleEnConst::Name && localeString != LocaleRuConst::Name)
        {
            m_locale = CreateLocale(StatsConst::Fields::Locale::Default);
            saveNeeded = true;
            spdlog::warn(
                "Locale field in \"{}\" is incorrect (\"{}\"), changing to default value (\"{}\")",
                m_filePath, localeString, m_locale->LocaleName()
            );
        }
        else
        {
            m_locale = CreateLocale(localeString);
        }

        int64_t timeoutDuration = statsJson[StatsConst::Fields::TimeoutDuration::Object];
        if (timeoutDuration <= 0 || timeoutDuration > StatsConst::Fields::TimeoutDuration::Max)
        {
            m_timeoutDuration = StatsConst::Fields::TimeoutDuration::Default;
            saveNeeded = true;
            spdlog::warn(
                "Timeout duration field in \"{}\" is incorrect ({}), changing to default value ({})",
                m_filePath, timeoutDuration, m_timeoutDuration
            );
        }
        else
        {
            m_timeoutDuration = timeoutDuration;
        }

        if (!saveNeeded)
            save();
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kc::Bot::Stats::Stats(): "
            "Couldn't parse stats file JSON [file: \"{}\", id: {}]",
            m_filePath, error.id
        ));
    }
}

void Bot::Stats::changeLocale(Locale::Type locale)
{
    m_locale = CreateLocale(locale);
    save();
}

void Bot::Stats::changeTimeoutDuration(int64_t timeoutDuration)
{
    if (timeoutDuration <= 0 || timeoutDuration > StatsConst::Fields::TimeoutDuration::Max)
    {
        throw std::invalid_argument(fmt::format(
            "kc::Bot::Stats::changeTimeoutDuration(): "
            "Invalid timeout duration [timeout duration: {}]",
            timeoutDuration
        ));
    }

    m_timeoutDuration = static_cast<uint32_t>(timeoutDuration);
    save();
}

} // namespace kc
