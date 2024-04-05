// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/utility.hpp"
#include "youtube/video.hpp"
using namespace kc;

static void PrintVideoInfo(const std::string& videoId)
{
    Youtube::Video video(videoId);
    fmt::print("{:<14} {}\n", "ID:", video.id());
    fmt::print("{:<14} {}\n", "Watch URL:", video.watchUrl());
    fmt::print("{:<14} {}\n", "Title:", video.title());
    fmt::print("{:<14} {}\n", "Author:", video.author());
    fmt::print("{:<14} {}\n", "Thumbnail URL:", video.thumbnailUrl());
    fmt::print("{:<14} {}\n", "Duration:", Utility::NiceString(video.duration()));
    fmt::print("{:<14} {}\n", "View count:", Utility::NiceString(video.viewCount()));
    fmt::print("{:<14} {}\n", "Category:", video.category());
    fmt::print("{:<14} {}\n", "Upload date:", Utility::NiceString(video.uploadDate()));
    fmt::print("{:<14} {}\n", "Chapters:", video.chapters().size());
}

int main()
{
    try
    {
        PrintVideoInfo("WVOH00wVFbc");
    } 
    catch (const Youtube::YoutubeError& error)
    {
        std::cout << fmt::format("YouTube error: {}\n", error.what());
    }
    catch (const Youtube::LocalError& error)
    {
        std::cout << fmt::format("Local error: {}\n", error.what());
    }
}
