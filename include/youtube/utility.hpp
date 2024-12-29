#pragma once

// STL modules
#include <string>

// Library nlohmann/json
#include <nlohmann/json.hpp>

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Youtube
{
    namespace Utility
    {
        /// @brief Extract item thumbnail URL from API response JSON thumbnails object
        /// @param thumbnailsObject API response JSON thumbnails object
        /// @return Extracted thumbnail URL
        std::string ExtractThumbnailUrl(const json& thumbnailsObject);

        /// @brief Extract string from API response JSON string object
        /// @param stringObject API response JSON string object
        /// @return Extracted string
        std::string ExtractString(const json& stringObject);

        /// @brief Extract item view count from API response view count string
        /// @param viewCountString API response view count string
        /// @return Extracted view count
        uint64_t ExtractViewCount(std::string viewCountString);

        /// @brief Extract item view count from API response JSON view count object
        /// @param viewCountObject API response JSON view count object
        /// @return Extracted view count
        uint64_t ExtractViewCount(const json& viewCountObject);
    }
}

} // namespace kb
