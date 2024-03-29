#include "youtube/search.hpp"

namespace kc {

static std::vector<Youtube::Item> ParseSearchContents(const json& contentsObject)
{
    std::vector<Youtube::Item> results;
    results.reserve(contentsObject.size());

    for (const json& contentObject : contentsObject)
    {
        if (contentObject.contains("compactVideoRenderer"))
        {
            Youtube::Video video(contentObject["compactVideoRenderer"]);
            switch (video.type())
            {
                case Youtube::Video::Type::Livestream:
                case Youtube::Video::Type::Upcoming:
                    continue;
            }

            results.emplace_back(std::in_place_type<Youtube::Video>, std::move(video));
            continue;
        }

        if (contentObject.contains("compactPlaylistRenderer"))
        {
            results.emplace_back(std::in_place_type<Youtube::Playlist>, contentObject["compactPlaylistRenderer"]);
            continue;
        }
    }

    results.shrink_to_fit();
    return results;
}

Youtube::SearchResult Youtube::Search(const std::string& query)
{
    if (query.find_first_not_of(' ') == std::string::npos)
    {
        throw std::invalid_argument(fmt::format(
            "kc::Youtube::Search(): [query]: \"{}\": Query is empty",
            query
        ));
    }

    // "tv_embedded" client is used for search because it's response is the lightest.
    Curl::Response searchResponse = Client::Instance->requestApi(Client::Type::TvEmbedded, "search", { {"query", query} });
    if (searchResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Search(): "
            "Couldn't get API response [query: \"{}\", client: \"tv_embedded\", response code: {}]",
            query, searchResponse.code
        ));
    }

    try
    {
        json contentsObject = json::parse(searchResponse.data)["contents"]
            ["sectionListRenderer"]["contents"][0]["itemSectionRenderer"]["contents"];
        return { SearchResult::Type::Search, query, ParseSearchContents(contentsObject) };
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Search(): "
            "Couldn't parse API response JSON [query: \"{}\", client: \"tv_embedded\", id: {}]",
            query, error.id
        ));
    }
}

Youtube::SearchResult Youtube::Related(const std::string& videoId)
{
    if (!boost::regex_match(videoId, boost::regex(VideoConst::ValidateId)))
    {
        throw std::invalid_argument(fmt::format(
            "kc::Youtube::Related(): [videoId]: \"{}\": Not a valid video ID",
            videoId
        ));
    }

    // "tv_embedded" client is used for related search because it's response is the lightest.
    Curl::Response nextResponse = Client::Instance->requestApi(Client::Type::TvEmbedded, "next", { {"videoId", videoId } });
    if (nextResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Related(): "
            "Couldn't get API response [video: \"{}\", client: \"tv_embedded\", response code: {}]",
            videoId, nextResponse.code
        ));
    }

    try
    {
        json contentsObject = json::parse(nextResponse.data)["contents"]["singleColumnWatchNextResults"]["results"]
            ["results"]["contents"][2]["shelfRenderer"]["content"]["horizontalListRenderer"]["items"];
        return { SearchResult::Type::Related, videoId, ParseSearchContents(contentsObject) };
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Related(): "
            "Couldn't parse API response JSON [video: \"{}\", client: \"tv_embedded\", id: {}]",
            videoId, error.id
        ));
    }
}

} // namespace kc
