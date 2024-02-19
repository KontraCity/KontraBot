#pragma once

// STL modules
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

// Library nlohmann/json
#include <nlohmann/json.hpp>

// Library Boost.Regex
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "youtube/item.hpp"
#include "youtube/video.hpp"

namespace kc {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Youtube
{
    struct SearchResult
    {
        enum class Type
        {
            Search,
            Related,
        };

        Type type;
        std::string query;
        std::vector<Item> items;
    };

    /// @brief Perform YouTube search
    /// @param query Search query
    /// @throw std::invalid_argument if [query] is empty
    /// @throw std::runtime_error if internal error occurs
    /// @return Search results
    SearchResult Search(const std::string& query);

    /// @brief Perform YouTube video related search
    /// @param videoId ID of the video
    /// @throw std::invalid_argument if [videoId] is not a valid video ID
    /// @throw std::runtime_error if internal error occurs
    /// @return Related search results
    SearchResult Related(const std::string& videoId);
}

} // namespace kc
