#include "youtube/utility.hpp"

namespace kc {

std::string Youtube::Utility::ExtractThumbnailUrl(const json& thumbnailsObject)
{
    int bestResolution = 0;
    std::string bestThumbnailUrl;
    for (const json& thumbnailObject : thumbnailsObject)
    {
        int currentResolution = thumbnailObject.at("width").get<int>() * thumbnailObject.at("height").get<int>();
        if (currentResolution > bestResolution)
        {
            bestResolution = currentResolution;
            bestThumbnailUrl = thumbnailObject.at("url");
        }
    }
    return bestThumbnailUrl;
}

std::string Youtube::Utility::ExtractString(const json& stringObject)
{
    if (stringObject.contains("simpleText"))
        return stringObject["simpleText"];

    std::string string;
    for (const json& runObject : stringObject.at("runs"))
        string += runObject.at("text");
    return string;
}

uint64_t Youtube::Utility::ExtractViewCount(std::string viewCountString)
{
    /*
    *   If the item has no views, YouTube API will return "No views" as a view count string instead of "0".
    *   This has to be checked to ensure that std::stoull() doesn't fail.
    */
    if (viewCountString == "No views")
        return 0;

    viewCountString.erase(
        std::remove_if(
            viewCountString.begin(),
            viewCountString.end(),
            [](char character) { return (character == ','); }
        ),
        viewCountString.end()
    );
    return std::stoull(viewCountString);
}

uint64_t Youtube::Utility::ExtractViewCount(const json& viewCountObject)
{
    std::string viewCountString = ExtractString(viewCountObject);
    return ExtractViewCount(viewCountString);
}

} // namespace kc
