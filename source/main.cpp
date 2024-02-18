// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/utility.hpp"
#include "youtube/video.hpp"
using namespace kc;

static void PrintVideoInfo(const Youtube::Video video)
{
    fmt::print("{: <15}{}\n", "ID:", video.id());
    fmt::print("{: <15}{}\n", "Title:", video.title());
    fmt::print("{: <15}{}\n", "Author:", video.author());
    fmt::print("{: <15}{}\n", "Thumbnail URL:", video.thumbnailUrl());
    fmt::print("{: <15}{}\n", "Duration:", Utility::ToString(video.duration()));
    fmt::print("{: <15}{}\n", "View count:", Utility::ToString(video.viewCount()));
    fmt::print("{: <15}{}\n", "Category:", video.category());
    fmt::print("{: <15}{}\n", "Upload date:", Utility::ToString(video.uploadDate()));
    
    std::cout << "Chapters:\n";
    if (video.chapters().empty())
    {
        std::cout << "No chapters.\n";
        return;
    }

    fmt::print("| {: <13}{}\n", "Timestamp", "Name");
    for (const Youtube::Video::Chapter& chapter : video.chapters())
        fmt::print("| {: <13}{}\n", Utility::ToString(chapter.timestamp), chapter.name);
}

int main()
{
    Youtube::Video video(std::string("WVOH00wVFbc"));
    PrintVideoInfo(video);
}
