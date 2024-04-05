// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/utility.hpp"
#include "youtube/video.hpp"
#include "youtube/playlist.hpp"
using namespace kc;

static void PrintVideoInfo(const std::string& idUrl)
{
    Youtube::Video video(idUrl);
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

static void PrintPlaylistInfo(const std::string& idUrl)
{
    Youtube::Playlist playlist(idUrl);
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
}

int main()
{
    try
    {
        PrintPlaylistInfo("https://www.youtube.com/playlist?list=PLn4GvABOzCQursVQ7qMU9CkNaKz4RgrVM");
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
