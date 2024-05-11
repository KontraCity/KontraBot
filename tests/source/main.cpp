// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "video.hpp"
using namespace kct;

enum class ParseResult
{
    None,       // Parsing error occured
    ShowHelp,   // Help message was requested
    TestVideo,  // Video module testing was requested
};

/// @brief Parse commmandline arguments
/// @param argc Count of arguments
/// @param argv Values of arguments
/// @return Parse result
static ParseResult ParseOptions(int argc, char** argv)
{
    ParseResult parseResult = ParseResult::None;
    for (int index = 1; index < argc; ++index)
    {
        std::string option = argv[index];
        if (parseResult != ParseResult::None)
        {
            fmt::print("Ignoring option: \"{}\"\n", option);
            continue;
        }

        if (option == "-h" || option == "--help")
        {
            parseResult = ParseResult::ShowHelp;
            continue;
        }

        if (option == "-v" || option == "--video")
        {
            parseResult = ParseResult::TestVideo;
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

    if (parseResult == ParseResult::None)
    {
        fmt::print(
            "Arguments required\n"
            "See {} --help\n",
            argv[0]
        );
    }
    return parseResult;
}

/// @brief Show help message
/// @param executableName Executable name
static void ShowHelpMessage(const char* executableName)
{
    fmt::print(
        "KontraBot tests usage: {} OPTIONS\n"
        "Possible options:\n"
        "    -h, --help\t\tShow this help message and exit\n"
        "    -v, --video\t\tTest YouTube video module\n"
        "Only one option may be passed. All others will be ignored.\n",
        executableName
    );
}

int main(int argc, char** argv)
{
    ParseResult parseResult = ParseOptions(argc, argv);
    switch (parseResult)
    {
        case ParseResult::None:
            return 1;
        case ParseResult::ShowHelp:
            ShowHelpMessage(argv[0]);
            return 0;
        case ParseResult::TestVideo:
            return Video::Test();
    }
}
