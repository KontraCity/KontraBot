#pragma once

// STL modules
#include <string>
#include <sstream>
#include <stdexcept>

// Library nlohmann/json
#include <nlohmann/json.hpp>

// Boost libraries
#include <boost/date_time.hpp>
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "youtube/client.hpp"
#include "youtube/error.hpp"
#include "youtube/utility.hpp"

namespace kc {

/* Namespace aliases and imports */
using nlohmann::json;
namespace dt = boost::gregorian;
namespace pt = boost::posix_time;

namespace Youtube
{
    namespace VideoConst
    {
        // Check if a string is a valid YouTube video ID
        constexpr const char* ValidateId = R"(^([^"&?\/\s]{11})$)";

        // Extract YouTube video ID from video watch URL
        // [C1]: Video ID
        constexpr const char* ExtractId = R"(youtu(?:be\.com|\.be)\/(?:(?:watch(?:_popup)?\?v=)|(?:embed\/)|(?:live\/)|(?:shorts\/))?([^"&?\/\s]{11}))";
    }

    class Video
    {
    public:
        enum class Type
        {
            /*
            *       Normal YouTube video that can be a:
            *           - Traditional video;
            *           - YouTube shorts video;
            *           - Livestream recording.
            *       All info fields are available and audio can be extracted.
            */
            Normal,

            /*
            *       YouTube livestream.
            *       It is not supported and can't be played.
            */
            Livestream,

            /*
            *       Premiered YouTube video that has a reveal date and will be:
            *           -> Revealed as a livestream;
            *           -> Converted to normal YouTube video after the reveal.
            *       It is not supported and can't be played.
            */
            Premiere,
        };

        struct Chapter
        {
            std::string name;
            pt::time_duration timestamp;
        };

    private:
        std::string m_id;
        std::string m_title;
        std::string m_author;
        std::string m_thumbnailUrl;
        pt::time_duration m_duration;
        uint64_t m_viewCount = 0;
        Type m_type = Type::Normal;

        bool m_optionalKnown = false;
        std::string m_category;
        dt::date m_uploadDate;
        std::vector<Chapter> m_chapters;

    private:
        /// @brief Check API response video playability status
        /// @param playabilityStatusObject API response JSON playability status object
        /// @throw kc::Youtube::YoutubeError if YouTube error is found
        void checkPlayabilityStatus(const json& playabilityStatusObject);

        /// @brief Parse API response video duration
        /// @param videoInfoObject API response JSON video info object
        /// @throw std::runtime_error if internal error occurs
        /// @return True if successfully extracted duration, false if the video is a livestream
        bool parseDuration(const json& videoInfoObject);

        /// @brief Parse API response video view count
        /// @param videoInfoObject API response JSON video info object
        void parseViewCount(const json& videoInfoObject);

        /// @brief Parse API response upload date string
        /// @param uploadDateString API response upload date string
        void parseUploadDate(std::string uploadDateString);

        /// @brief Parse API response video description to chapters
        /// @param description API response video description
        void parseChapters(const std::string& description);

        /// @brief Download video info
        /// @throw std::runtime_error if internal error occurs
        /// @throw kc::Youtube::YoutubeError if YouTube error occurs
        void downloadInfo();

        /// @brief Check optional fields availability
        /// @throw std::runtime_error if internal error occurs
        /// @throw kc::Youtube::YoutubeError if YouTube error occurs
        void checkOptional() const;

    public:
        /// @brief Get video info
        /// @param idUrl Video ID or watch URL
        /// @throw std::invalid_argument if [idUrl] is not a valid video ID or watch URL
        /// @throw std::runtime_error if internal error occurs
        /// @throw kc::Youtube::YoutubeError if YouTube error occurs
        Video(const std::string& idUrl);

        /// @brief Parse video info
        /// @param videoInfoObject API response JSON video info object
        Video(const json& videoInfoObject);

        /// @brief Get video ID
        /// @return Video ID
        inline const std::string& id() const
        {
            return m_id;
        }

        /// @brief Get video watch URL
        /// @return Video watch URL
        inline std::string watchUrl() const
        {
            return "https://www.youtube.com/watch?v=" + m_id;
        }

        /// @brief Get video watch URL with timestamp
        /// @param timestamp Timestamp to include in the URL
        /// @return Video watch URL with timestamp
        inline std::string watchUrl(pt::time_duration timestamp) const
        {
            return fmt::format(
                "https://www.youtube.com/watch?v={}&t={}",
                m_id,
                timestamp.total_seconds()
            );
        }

        /// @brief Get video title
        /// @return Video title
        inline const std::string& title() const
        {
            return m_title;
        }

        /// @brief Get video author
        /// @return Video author
        inline const std::string& author() const
        {
            return m_author;
        }

        /// @brief Get video thumbnail URL
        /// @return Video thumbnail URL
        inline const std::string& thumbnailUrl() const
        {
            return m_thumbnailUrl;
        }

        /// @brief Get video duration
        /// @return Video duration
        inline pt::time_duration duration() const
        {
            return m_duration;
        }

        /// @brief Get video view count
        /// @return Video view count
        inline uint64_t viewCount() const
        {
            return m_viewCount;
        }

        /// @brief Get video type
        /// @return Video type
        inline Type type() const
        {
            return m_type;
        }

        /// @brief Get video category: optional info may be downloaded
        /// @throw std::runtime_error if internal error occurs
        /// @throw kc::Youtube::YoutubeError if YouTube error occurs
        /// @return Video category
        inline const std::string& category() const
        {
            checkOptional();
            return m_category;
        }

        /// @brief Get video upload date: optional info may be downloaded
        /// @throw std::runtime_error if internal error occurs
        /// @throw kc::Youtube::YoutubeError if YouTube error occurs
        /// @return Video upload date
        inline dt::date uploadDate() const
        {
            checkOptional();
            return m_uploadDate;
        }

        /// @brief Get video chapters: optional info may be downloaded
        /// @throw std::runtime_error if internal error occurs
        /// @throw kc::Youtube::YoutubeError if YouTube error occurs
        /// @return Video chapters
        inline const std::vector<Chapter>& chapters() const
        {
            checkOptional();
            return m_chapters;
        }
    };
}

} // namespace kc
