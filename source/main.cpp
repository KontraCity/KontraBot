// STL modules
#include <iostream>

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/config.hpp"
using namespace kc;

/// @brief Initialize config
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

int main()
{
    Bot::Config::Pointer config = Init();
    if (!config)
        return 1;

    std::cout << fmt::format("Discord bot API token: \"{}\"\n", config->discordBotApiToken());
    return 0;
}
