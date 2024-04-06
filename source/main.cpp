// STL modules
#include <iostream>

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale_en.hpp"
#include "bot/locale/locale_ru.hpp"
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

static void PrintLocaleInfo(const Bot::Locale::Pointer& locale)
{
    fmt::print("Locale type: {}\n", static_cast<int>(locale->type()));
    fmt::print("Locale name: {}\n", locale->name());
    fmt::print("Locale long name: {}\n\n", locale->longName());
}

int main()
{
    Bot::Config::Pointer config = Init();
    if (!config)
        return 1;
    PrintLocaleInfo(std::make_unique<Bot::LocaleEn>());
    PrintLocaleInfo(std::make_unique<Bot::LocaleRu>());
}
