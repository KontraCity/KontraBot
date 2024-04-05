// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/stopwatch.hpp"
#include "common/utility.hpp"
#include "youtube/item.hpp"
#include "youtube/search.hpp"
using namespace kc;

static void PrintInfo(const Youtube::Item& item)
{
    switch (item.type())
    {
        case Youtube::Item::Type::Video:
        {
            const Youtube::Video& video = std::get<Youtube::Video>(item);
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
            return;
        }
        case Youtube::Item::Type::Playlist:
        {
            const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
            fmt::print("{:<14} {}\n", "ID:", playlist.id());
            fmt::print("{:<14} {}\n", "View URL:", playlist.viewUrl());
            fmt::print("{:<14} {}\n", "Title:", playlist.title());
            fmt::print("{:<14} {}\n", "Author:", playlist.author());
            fmt::print("{:<14} {}\n", "Thumbnail URL:", playlist.thumbnailUrl());
            fmt::print("{:<14} {}\n", "Video count:", Utility::NiceString(playlist.videoCount()));
            fmt::print("{:<14} {}\n", "View count:", Utility::NiceString(playlist.viewCount()));
            fmt::print("{:<14} {}\n", "Videos hidden:", playlist.videosHidden() ? "true" : "false");

            std::cout << "\nPrinting playlist videos:\n";
            fmt::print("{:<5} {}\n", "Index", "Title");
            for (Youtube::Playlist::Iterator iterator = playlist.begin(); iterator; ++iterator)
                fmt::print("{:<5} {}\n", iterator.index() + 1, iterator->title());
            return;
        }
    }
}

int main()
{
    try
    {
        for (int iteration = 0; iteration < 15; ++iteration)
        {
            Stopwatch stopwatch;
            Youtube::Results results = Youtube::Search("music");
            fmt::print("Iteration {}: {} results, {} ms\n", iteration, results.size(), stopwatch.elapsed<Stopwatch::Milliseconds>());
        }
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
