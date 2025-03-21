#include <dpp/dpp.h>

#include <spdlog/sinks/stdout_color_sinks.h>

// STL modules
#include <filesystem>

// Library {fmt}
#include <fmt/format.h>

#include <ytcpp/core/logger.hpp>
#include <ytcpp/innertube.hpp>

// Custom modules
#include "bot/bot.hpp"
#include "bot/info.hpp"
#include "core/config.hpp"
#include "core/utility.hpp"
using namespace kb;

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
/// @return Commandline arguments parse result
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
/// @param result Commandline arguments parse result
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
    if (std::filesystem::is_regular_file(ConfigConst::ConfigFile))
    {
        fmt::print(
            "Configuration file \"{}\" already exists.\n"
            "Delete it first to confirm that you don't care about its contents.\n",
            ConfigConst::ConfigFile
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
        Config::GenerateSampleFile();
    }
    catch (...)
    {
        fmt::print(
            "Couldn't create configuration file \"{}\".\n"
            "Please check permissions.\n",
            ConfigConst::ConfigFile
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
        "Please configure the file before starting KontraBot.\n",
        ConfigConst::ConfigFile, Bot::InfoConst::InfoDirectory
    );
    return 0;
}

/// @brief Check singletons initialization
/// @param result Commandline arguments parse result
/// @return True if all singletons initialized successfully
static bool CheckSingletons(const ParseResult& result)
{
    spdlog::logger logger = Utility::CreateLogger("init", result.forceColor);

    if (!Config::GetError().empty())
    {
        logger.error("Configuration error: {}", Config::GetError());
        logger.info("Hint: Check configuration file \"{}\"", ConfigConst::ConfigFile);
        logger.info("Hint: You can generate necessary files by running {} --generate", result.executableName);
        return false;
    }

    return true;
}

static void YtcppInit() {
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sink->set_pattern("[%^%d.%m.%C %H:%M:%S %L%$] [%n] %v");
    sink->set_color_mode(spdlog::color_mode::always);
    ytcpp::Logger::Sinks().push_back(std::move(sink));
    ytcpp::Logger::SetLevel(spdlog::level::debug);
    ytcpp::Innertube::Authorize();
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

    bool singletonsCheck = CheckSingletons(result);
    if (!singletonsCheck)
        return 1;

    if (result.result == ParseResult::Result::Register)
    {
        Bot::Bot bot(true);
        return 0;
    }

    fmt::print(
        "Welcome to KontraBot NG\n"
        "GitHub repository: https://github.com/KontraCity/KontraBot\n"
    );

    YtcppInit();
    Bot::Bot bot;
    bot.start(dpp::st_wait);
}
