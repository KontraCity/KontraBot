// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/utility.hpp"
#include "youtube/item.hpp"
#include "youtube/search.hpp"
#include "youtube/extractor.hpp"
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
                fmt::print("| | {: >5} {}\n", (iterator.index() + 1), iterator->title());
            break;
        }
    }
}

static void PrintSearchResult(const Youtube::SearchResult& result)
{
    switch (result.type)
    {
        case Youtube::SearchResult::Type::Search:
            fmt::print("Search \"{}\":\n", result.query);
            break;
        case Youtube::SearchResult::Type::Related:
            fmt::print("Related search \"{}\":\n", result.query);
            break;
    }

    if (result.items.empty())
    {
        std::cout << "| No results.\n";
        return;
    }
     
    fmt::print("| {: >5} {: <8} {}\n", "Index", "Type", "Title");
    for (size_t index = 0; index < result.items.size(); ++index)
    {
        if (result.items[index].type() == Youtube::Item::Type::Video)
            fmt::print("| {: >5} {: <8} {}\n", (index + 1), "Video", std::get<Youtube::Video>(result.items[index]).title());
        else
            fmt::print("| {: >5} {: <8} {}\n", (index + 1), "Playlist", std::get<Youtube::Playlist>(result.items[index]).title());
    }
}

int main()
{
    fmt::print("{: >9} {}\n", "Timestamp", "Length");
    Youtube::Extractor extractor("WVOH00wVFbc");
    while (true)
    {
        Youtube::Extractor::Frame frame = extractor.extractFrame();
        if (frame.data.empty())
            break;
        fmt::print("{: >9} {} bytes\n", Utility::ToString(pt::time_duration(0, 0, 0, frame.timestamp * 1000)), frame.data.size());
    }
}
