#include "youtube/search.hpp"

// STL modules
#include <stdexcept>

// Library nlohmann/json
#include <nlohmann/json.hpp>

// Library Boost.Regex
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "youtube/client.hpp"

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

/// @brief Parse API response JSON contents object to results list
/// @param contentsObject API response JSON contents object
/// @param type Results list type
/// @param query Results list query
/// @return Results list
static Youtube::Results ParseSearchContents(const json& contentsObject, Youtube::Results::Type type, const std::string& query)
{
    Youtube::Results results(type, query);
    results.reserve(contentsObject.size());

    for (const json& contentObject : contentsObject)
    {
        /*
        *   Search contents may contain movies and shows.
        *   They don't have author field and can't be played anyway.
        *   Ignore them.
        */
        if (contentObject.contains("compactVideoRenderer") && contentObject["compactVideoRenderer"].contains("shortBylineText"))
        {
            Youtube::Video video(contentObject["compactVideoRenderer"]);
            if (video.type() == Youtube::Video::Type::Normal)
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

Youtube::Results::Results(Type type, const std::string& query)
    : m_type(type)
    , m_query(query)
{}

Youtube::Results Youtube::Search(const std::string& query)
{
    if (query.find_first_not_of(' ') == std::string::npos)
    {
        throw std::invalid_argument(fmt::format(
            "kb::Youtube::Search(): [query]: \"{}\": Query is empty",
            query
        ));
    }

    // TV embedded client is used for search because its response is the lightest.
    Curl::Response searchResponse = Client::Instance->requestApi(Client::Type::TvEmbedded, "search", { {"query", query} }, true);
    if (searchResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Search(): "
            "Couldn't get API response [query: \"{}\", client: \"tv_embedded\", response code: {}]",
            query, searchResponse.code
        ));
    }

    try
    {
        json contentsObject = json::parse(searchResponse.data).at("contents")
            .at("sectionListRenderer").at("contents").at(0).at("itemSectionRenderer").at("contents");
        return ParseSearchContents(contentsObject, Results::Type::Search, query);
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Search(): "
            "Couldn't parse API response JSON [query: \"{}\", client: \"tv_embedded\", id: {}]",
            query, error.id
        ));
    }
}

Youtube::Results Youtube::Related(const std::string& videoId)
{
    if (!boost::regex_match(videoId, boost::regex(VideoConst::ValidateId)))
    {
        throw std::invalid_argument(fmt::format(
            "kb::Youtube::Related(): [videoId]: \"{}\": Not a valid video ID",
            videoId
        ));
    }

    // TV embedded client is used for related search because its response is the lightest.
    Curl::Response nextResponse = Client::Instance->requestApi(Client::Type::TvEmbedded, "next", { {"videoId", videoId } }, true);
    if (nextResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Related(): "
            "Couldn't get API response [video: \"{}\", client: \"tv_embedded\", response code: {}]",
            videoId, nextResponse.code
        ));
    }

    try
    {
        json contentsObject = json::parse(nextResponse.data).at("contents").at("singleColumnWatchNextResults").at("results")
            .at("results").at("contents").at(2).at("shelfRenderer").at("content").at("horizontalListRenderer").at("items");
        return ParseSearchContents(contentsObject, Results::Type::Related, videoId);
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Related(): "
            "Couldn't parse API response JSON [video: \"{}\", client: \"tv_embedded\", id: {}]",
            videoId, error.id
        ));
    }
}

} // namespace kb
