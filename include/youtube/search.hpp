#pragma once

// STL modules
#include <string>
#include <vector>
#include <stdexcept>

// Library nlohmann/json
#include <nlohmann/json.hpp>

// Library Boost.Regex
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "youtube/item.hpp"

namespace kc {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Youtube
{
    class Results : public std::vector<Item>
    {
    public:
        enum class Type
        {
            Search,     // Results list contains items from query search
            Related,    // Results list contains items related to query video
        };

    private:
        Type m_type;
        std::string m_query;

    public:
        /// @brief Create results list
        /// @param type Results list type
        /// @param query Results list query
        Results(Type type, const std::string& query);

        /// @brief Get results list type
        /// @return Results list type
        inline Type type() const
        {
            return m_type;
        }

        /// @brief Get results list query
        /// @return Results list query
        inline const std::string& query() const
        {
            return m_query;
        }
    };
    
    /// @brief Perform YouTube search
    /// @param query Search query
    /// @throw std::invalid_argument if [query] is empty
    /// @throw std::runtime_error if internal error occurs
    /// @return Search results
    Results Search(const std::string& query);

    /// @brief Perform video related search
    /// @param videoId ID of the video in question
    /// @throw std::invalid_argument if [videoId] is not a valid video ID
    /// @throw std::runtime_error if internal error occurs
    /// @return Related search results
    Results Related(const std::string& videoId);
}

} // namespace kc
