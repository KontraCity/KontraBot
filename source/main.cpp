// STL modules
#include <filesystem>

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/info/info.hpp"
#include "bot/bot.hpp"
#include "bot/config.hpp"
using namespace kc;

enum class ParseResult
{
    None,       // Parsing error occured
    Help,       // Help message was requested
    Generate,   // Necessary files generation was requested
    Register,   // Commands registering was requested
    Start,      // Normal bot start was requested
};

static ParseResult ParseOptions(int argc, char** argv)
{
    ParseResult parseResult = ParseResult::Start;
    for (int index = 1; index < argc; ++index)
    {
        std::string option = argv[index];
        if (parseResult != ParseResult::Start)
        {
            fmt::print("Ignoring option: \"{}\"\n", option);
            continue;
        }

        if (option == "-h" || option == "--help")
        {
            parseResult = ParseResult::Help;
            continue;
        }

        if (option == "-g" || option == "--generate")
        {
            parseResult = ParseResult::Generate;
            continue;
        }

        if (option == "-r" || option == "--register")
        {
            parseResult = ParseResult::Register;
            continue;
        }

        fmt::print(
            "Unknown option: \"{}\"\n"
            "See {} --help\n",
            option,
            argv[0]
        );
        return ParseResult::None;
    }
    return parseResult;
}

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

/// @brief Show help message
/// @param executableName Executable name
static void ShowHelpMessage(const char* executableName)
{
    fmt::print(
        "KontraBot usage: {} [OPTIONS]\n"
        "Possible options:\n"
        "    No options\t\tStart bot normally\n"
        "    -h, --help\t\tShow this help message and exit\n"
        "    -g, --generate\tGenerate necessary files and exit\n"
        "    -r, --register\tRegister slashcommands and exit\n"
        "Only one option may be passed. All others will be ignored.\n",
        executableName
    );
}

/// @brief Generate necessary files
/// @return Executable exit code
static int GenerateFiles()
{
    if (std::filesystem::is_regular_file(Bot::ConfigConst::ConfigFile))
    {
        fmt::print(
            "Configuration file \"{}\" already exists.\n"
            "Delete it first to confirm that you don't care about its contents.\n",
            Bot::ConfigConst::ConfigFile
        );
        return 1;
    }

    if (std::filesystem::is_directory(Bot::InfoConst::InfoDirectory))
    {
        fmt::print(
            "Information directory \"{}/\" already exists.\n"
            "Delete it first to confirm that you don't care about its contents.\n",
            Bot::InfoConst::InfoDirectory
        );
        return 1;
    }

    try
    {
        Bot::Config::GenerateSampleFile();
    }
    catch (...)
    {
        fmt::print(
            "Couldn't create configuration file \"{}\".\n"
            "Please check permissions.\n",
            Bot::ConfigConst::ConfigFile
        );
        return 1;
    }

    if (!std::filesystem::create_directory(Bot::InfoConst::InfoDirectory))
    {
        fmt::print(
            "Couldn't create information directory \"{}/\".\n"
            "Please check permissions.\n",
            Bot::InfoConst::InfoDirectory
        );
        return 1;
    }

    fmt::print(
        "Configuration file \"{}\" and information directory \"{}/\" were created.\n"
        "Please configure the bot before starting it.\n",
        Bot::ConfigConst::ConfigFile, Bot::InfoConst::InfoDirectory
    );
    return 0;
}

int main(int argc, char** argv)
{
    ParseResult parseResult = ParseOptions(argc, argv);
    switch (parseResult)
    {
        case ParseResult::None:
            return 1;
        case ParseResult::Help:
            ShowHelpMessage(argv[0]);
            return 0;
        case ParseResult::Generate:
            return GenerateFiles();
        default:
            break;
    }

    Bot::Config::Pointer config = Init();
    if (!config)
        return 1;

    fmt::print(
        "Welcome to KontraBot NG\n"
        "GitHub repository: https://github.com/KontraCity/KontraBot\n"
    );

    if (parseResult == ParseResult::Register)
    {
        Bot::Bot bot(config, true);
        return 0;
    }

    Bot::Bot bot(config);
    bot.start(false);
}
