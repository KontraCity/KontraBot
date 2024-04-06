// STL modules
#include <iostream>

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/config.hpp"
#include "bot/signal.hpp"
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

static void PrintSignalInfo(const std::string& signalString)
{
    Bot::Signal signal(signalString);
    fmt::print("Signal type: {}\n", static_cast<int>(signal.type()));
    fmt::print("Signal data: {}\n", signal.data());
}

int main()
{
    Bot::Config::Pointer config = Init();
    if (!config)
        return 1;

    Bot::Signal signal(Bot::Signal::Type::Played, "WVOH00wVFbc");
    PrintSignalInfo(signal);
}
