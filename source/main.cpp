// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "youtube/error.hpp"
using namespace kc;

int main()
{
    try
    {
        throw Youtube::YoutubeError(
            Youtube::YoutubeError::Type::LoginRequired,
            "s7_qI6_mIXc",
            "Video unavailable",
            "This video is private"
        );
    }
    catch (const Youtube::YoutubeError& error)
    {
        std::cout << fmt::format(
            "YouTube error: {}\n"
            "Type: {}\n"
            "Item ID: {}\n"
            "Reason: {}\n"
            "Subreason: {}\n",
            error.what(),
            static_cast<int>(error.type()),
            error.itemId(),
            error.reason(),
            error.subreason()
        );
    }

    std::cout << '\n';

    try
    {
        throw Youtube::LocalError(
            Youtube::LocalError::Type::LivestreamNotSupported,
            "jfKfPfyJRdk"
        );
    }
    catch (const Youtube::LocalError& error)
    {
        std::cout << fmt::format(
            "Local error: {}\n"
            "Type: {}\n"
            "Item ID: {}\n",
            error.what(),
            static_cast<int>(error.type()),
            error.itemId()
        );
    }
}
