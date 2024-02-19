// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/utility.hpp"
#include "youtube/video.hpp"
#include "youtube/playlist.hpp"
using namespace kc;

static void PrintVideoInfo(const Youtube::Video video)
{
    fmt::print("Video \"{}\":\n", video.id());
    fmt::print("| {: <15} {}\n", "Title:", video.title());
    fmt::print("| {: <15} {}\n", "Author:", video.author());
    fmt::print("| {: <15} {}\n", "Thumbnail URL:", video.thumbnailUrl());
    fmt::print("| {: <15} {}\n", "Duration:", Utility::ToString(video.duration()));
    fmt::print("| {: <15} {}\n", "View count:", Utility::ToString(video.viewCount()));
    fmt::print("| {: <15} {}\n", "Category:", video.category());
    fmt::print("| {: <15} {}\n", "Upload date:", Utility::ToString(video.uploadDate()));
    
    std::cout << "| Chapters:\n";
    if (video.chapters().empty())
    {
        std::cout << "| | No chapters.\n";
        return;
    }

    fmt::print("| | {: >9} {}\n", "Timestamp", "Name");
    for (const Youtube::Video::Chapter& chapter : video.chapters())
        fmt::print("| | {: >9} {}\n", Utility::ToString(chapter.timestamp), chapter.name);
}

static void PrintPlaylistInfo(const Youtube::Playlist& playlist)
{
    fmt::print("Playlist \"{}\":\n", playlist.id());
    fmt::print("| {: <30} {}\n", "Title:", playlist.title());
    fmt::print("| {: <30} {}\n", "Author:", playlist.author());
    fmt::print("| {: <30} {}\n", "Thumbnail URL:", playlist.thumbnailUrl());
    fmt::print("| {: <30} {}\n", "Video count:", Utility::ToString(playlist.videoCount()));
    fmt::print("| {: <30} {}\n", "View count:", Utility::ToString(playlist.viewCount()));
    fmt::print("| {: <30} {}\n", "Unavailable videos hidden?", (playlist.videosHidden() ? "Yes" : "No"));

    fmt::print("| {}\n", "Videos:");
    fmt::print("| | {: >5} {}\n", "Index", "Title");
    for (Youtube::Playlist::Iterator iterator = playlist.begin(); iterator; ++iterator)
        fmt::print("| | {: >5} {}\n", iterator.index(), iterator->title());
}

int main()
{
    Youtube::Video video(std::string("WVOH00wVFbc"));
    PrintVideoInfo(video);

    std::cout << '\n';

    Youtube::Playlist playlist(std::string("PLKUA473MWUv2jmkqIxzQR3YL4kuPArj4G"));
    PrintPlaylistInfo(playlist);
}
