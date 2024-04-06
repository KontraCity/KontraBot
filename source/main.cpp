// STL modules
#include <iostream>

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/info.hpp"
#include "bot/config.hpp"
using namespace kc;

/// @brief Initialize con fig
/// @return Initialized config
static Bot::Config::Pointer Init()
{
    try
    {
        return std::make_shared<Bot::Config>();
    }
    catch (const Bot::Config::Error& error)
    {
        spdlog::logger logger("init", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logger.error("Configuration error: {}", error.what());
        return {};
    }
}

static void PrintGuildInfo(dpp::snowflake guildId)
{
    Bot::Info info(guildId);
    const Bot::Settings& settings = info.settings();
    fmt::print("Locale: {}\n", settings.locale->longName());
    fmt::print("Timeout duration: {} seconds\n", settings.timeoutMinutes);

    const Bot::Stats& stats = info.stats();
    fmt::print("Sessions count: {}\n", stats.sessionsCount);
    fmt::print("Tracks played: {}\n", stats.tracksPlayed);
}

int main()
{
    Bot::Config::Pointer config = Init();
    if (!config)
        return 1;

    PrintGuildInfo(0);
}
