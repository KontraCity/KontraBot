// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/utility.hpp"
#include "youtube/item.hpp"
using namespace kc;

static void PrintItemInfo(const Youtube::Item& item)
{
    switch (item.type())
    {
        case Youtube::Item::Type::Video:
        {
            const Youtube::Video& video = std::get<Youtube::Video>(item);
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
                break;
            }

            fmt::print("| | {: >9} {}\n", "Timestamp", "Name");
            for (const Youtube::Video::Chapter& chapter : video.chapters())
                fmt::print("| | {: >9} {}\n", Utility::ToString(chapter.timestamp), chapter.name);
            break;
        }
        case Youtube::Item::Type::Playlist:
        {
            const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
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
            break;
        }
        default:
            break;
    }
}

int main()
{
    Youtube::Video video(std::string("WVOH00wVFbc"));
    PrintItemInfo(video);

    std::cout << '\n';

    Youtube::Playlist playlist(std::string("PLKUA473MWUv2jmkqIxzQR3YL4kuPArj4G"));
    PrintItemInfo(playlist);
}
