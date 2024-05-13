// STL modules
#include <filesystem>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/bot.hpp"
#include "bot/config.hpp"
#include "bot/info.hpp"
#include "common/utility.hpp"
using namespace kc;

struct ParseResult
{
    enum class Result
    {
        None,       // Parsing error occured
        ShowHelp,   // Help message was requested
        Generate,   // Necessary files generation was requested
        Register,   // Commands registering was requested
        Start,      // Normal bot start was requested
    };

    const char* executableName;
    Result result;
    bool forceColor;
};

/// @brief Parse commmandline arguments
/// @param argc Count of arguments
/// @param argv Values of arguments
/// @return Parse result
static ParseResult ParseOptions(int argc, char** argv)
{
    ParseResult result;
    result.executableName = argv[0];
    result.result = ParseResult::Result::Start;
    result.forceColor = false;

    for (int index = 1; index < argc; ++index)
    {
        std::string option = argv[index];

        if (option == "-fc" || option == "--force-color")
        {
            result.forceColor = true;
            continue;
        }

        if (result.result != ParseResult::Result::Start)
        {
            fmt::print("Ignoring option: \"{}\"\n", option);
            continue;
        }

        if (option == "-h" || option == "--help")
        {
            result.result = ParseResult::Result::ShowHelp;
            continue;
        }

        if (option == "-g" || option == "--generate")
        {
            result.result = ParseResult::Result::Generate;
            continue;
        }

        if (option == "-r" || option == "--register")
        {
            result.result = ParseResult::Result::Register;
            continue;
        }

        fmt::print(
            "Unknown option: \"{}\"\n"
            "See {} --help\n",
            option,
            result.executableName
        );

        result.result = ParseResult::Result::None;
        return result;
    }

    return result;
}

/// @brief Show help message
/// @param executableName Executable name
static int ShowHelpMessage(const ParseResult& result)
{
    fmt::print(
        "KontraBot usage: {} [OPTIONS]\n"
        "Available options:\n"
        "    (No options)\tStart bot normally\n"
        "    -fc, --force-color\tForce colored logs regardless of whether your tty supports them or not\n"
        "Unique options:\n"
        "    -h, --help\t\tShow this message and exit\n"
        "    -g, --generate\tGenerate necessary files and exit\n"
        "    -r, --register\tRegister slashcommands and exit\n"
        "Only one of the unique options may be passed at the same time. All others will be ignored.\n",
        result.executableName
    );
    return 0;
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

/// @brief Initialize config
/// @return Initialized config
static Bot::Config::Pointer Init(const ParseResult& result)
{
    spdlog::logger logger = Utility::CreateLogger("init", result.forceColor);
    try
    {
        return std::make_shared<Bot::Config>();
    }
    catch (const Bot::Config::Error& error)
    {
        logger.error("Configuration error: {}", error.what());
        logger.info("Hint: Check configuration file \"{}\"", Bot::ConfigConst::ConfigFile);
        logger.info("Hint: You can generate necessary files by running {} --generate", result.executableName);
        return {};
    }
}

int main(int argc, char** argv)
{
    ParseResult result = ParseOptions(argc, argv);
    switch (result.result)
    {
        case ParseResult::Result::None:
            return 1;
        case ParseResult::Result::ShowHelp:
            return ShowHelpMessage(result);
        case ParseResult::Result::Generate:
            return GenerateFiles();
        default:
            break;
    }

    Bot::Config::Pointer config = Init(result);
    if (!config)
        return 1;

    fmt::print(
        "Welcome to KontraBot NG\n"
        "GitHub repository: https://github.com/KontraCity/KontraBot\n"
    );

    if (result.result == ParseResult::Result::Register)
    {
        Bot::Bot bot(config, true);
        return 0;
    }

    Bot::Bot bot(config);
    bot.start(false);
}
