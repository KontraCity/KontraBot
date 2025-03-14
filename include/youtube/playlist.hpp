#pragma once

// STL modules
#include <string>
#include <vector>
#include <iterator>
#include <cstddef>

// Library nlohmann::json
#include <nlohmann/json.hpp>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "youtube/video.hpp"

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Youtube
{
    namespace PlaylistConst
    {
        // Check if a string is a valid YouTube playlist ID
        constexpr const char* ValidateId = R"(^(PL[^"&?\/\s]{16,32}$|^OLAK5uy_[^"&?\/\s]{33})$)";

        // Extract YouTube playlist ID from playlist view URL
        // [C1]: Playlist ID
        constexpr const char* ExtractId = R"(youtube\.com\/(?:playlist\?list=|watch\?v=[^"&?\/\s]{11}&list=)(PL[^"&?\/\s]{16,32}|OLAK5uy_[^"&?\/\s]{33}))";
    }

    class Playlist
    {
    public:
        class Iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = Video;
            using pointer = Video*;
            using reference = Video&;

        private:
            Playlist* m_root;
            pointer m_video;
            size_t m_index;

        public:
            /// @brief Create invalid playlist iterator
            Iterator();

            /// @brief Create playlist iterator
            /// @param root Playlist to iterate
            /// @param index Index of video to initialize iterator for
            Iterator(Playlist* root, size_t index = 0);

        public:
            /// @brief Get current video with playlist watch URL
            /// @throw std::invalid_argument if iterator is invalid
            /// @return Current video with playlist watch URL
            inline std::string watchUrl() const
            {
                if (!m_video)
                    throw std::invalid_argument("kb::Playlist::Iterator::watchUrl(): Iterator is invalid");
                return fmt::format("https://www.youtube.com/watch?v={}&list={}", m_video->id(), m_root->id());
            }

        public:
            /// @brief Get current video pointer
            /// @return Current video pointer
            inline const pointer operator->() const
            {
                if (!m_video)
                    throw std::invalid_argument("kb::Playlist::Iterator::operator->(): Iterator is invalid");
                return m_video;
            }

            /// @brief Get current video
            /// @return Current video
            inline const reference operator*() const
            {
                if (!m_video)
                    throw std::invalid_argument("kb::Playlist::Iterator::operator*(): Iterator is invalid");
                return *m_video;
            }

            /// @brief Get current video index
            /// @return Current video index
            inline size_t index() const
            {
                if (!m_video)
                    throw std::invalid_argument("kb::Playlist::Iterator::index(): Iterator is invalid");
                return m_index;
            }

            /// @brief Advance iterator forward
            /// @return Modified iterator
            inline Iterator& operator++()
            {
                if (m_video)
                    m_video = m_root->discoverVideo(++m_index);
                return *this;
            }

            /// @brief Advance iterator forward
            /// @return Previous version of iterator
            inline Iterator operator++(int)
            {
                ++(*this);
                Iterator previousIterator = *this;
                return --previousIterator;
            }

            /// @brief Advance iterator back
            /// @return Modified iterator
            inline Iterator& operator--()
            {
                if (m_index != 0)
                    m_video = m_root->discoverVideo(--m_index);
                return *this;
            }

            /// @brief Advance iterator back
            /// @return Previous version of iterator
            inline Iterator operator--(int)
            {
                --(*this);
                Iterator previousIterator = *this;
                return ++previousIterator;
            }

            /// @brief Check if iterator is valid
            /// @return True if iterator is valid
            inline operator bool() const
            {
                return static_cast<bool>(m_video);
            }

            /// @brief Check if two iterators are equal
            /// @param left First iterator
            /// @param right Second iterator
            /// @return True if iterators are equal
            friend inline bool operator==(const Iterator& left, const Iterator& right)
            {
                return (left.m_video == right.m_video);
            }

            /// @brief Check if two iterators are not equal
            /// @param left First iterator
            /// @param right Second iterator
            /// @return True if iterators are not equal
            friend inline bool operator!=(const Iterator& left, const Iterator& right)
            {
                return (left.m_video != right.m_video);
            }
        };

    private:
        // Common members
        std::string m_id;
        std::string m_title;
        std::string m_author;
        std::string m_thumbnailUrl;
        int m_videoCount = 0;

        // Optional members
        bool m_optionalKnown = false;
        uint64_t m_viewCount = 0;
        bool m_videosHidden = false;

        // Contents members
        std::vector<Video> m_videos;
        std::string m_continuationToken;

    public:
        /// @brief Get playlist info
        /// @param idUrl Playlist ID or view URL
        /// @throw std::invalid_argument if [idUrl] is not a valid playlist ID or view URL
        /// @throw std::runtime_error if internal error occurs
        /// @throw kb::Youtube::YoutubeError if YouTube playlist error occurs
        /// @throw kb::Youtube::LocalError if playlist is not supported
        Playlist(const std::string& idUrl);

        /// @brief Parse playlist info
        /// @param playlistInfoObject API response JSON playlist info object
        Playlist(const json& playlistInfoObject);
        
    private:
        /// @brief Check API response playlist alerts
        /// @param browseResponseJson API "browse" JSON response
        /// @throw kb::Youtube::YoutubeError if YouTube playlist error is found
        void checkAlerts(const json& browseResponseJson);

        /// @brief Parse API response playlist author
        /// @param playlistHeaderRendererObject API responsee JSON playlist header renderer object
        void parseAuthor(const json& playlistHeaderRendererObject);

        /// @brief Parse API response playlist thumbnail URL from playlist header renderer object
        /// @param playlistHeaderRendererObject API response JSON playlist header renderer object
        void parseThumbnailUrlHeaderRenderer(const json& playlistHeaderRendererObject);

        /// @brief Parse API response playlist thumbnail URL from playlist sidebar primary info renderer object
        /// @param playlistSidebarPrimaryInfoRendererObject API response JSON playlist sidebar primary info renderer object
        void parseThumbnailUrlSidebarRenderer(const json& playlistSidebarPrimaryInfoRendererObject);

        /// @brief Parse API response playlist videos
        /// @param videoContentsObject API response JSON video contents object
        /// @throw kb::Youtube::LocalError if all playlist items are not supported
        void parseVideos(const json& videoContentsObject);

        /// @brief Parse API response video count
        /// @param videoCountObject API response JSON video count object
        void parseVideoCount(const json& videoCountObject);

        /// @brief Discover playlist video
        /// @param index Video index
        /// @throw std::runtime_error if internal error occurs
        /// @return Discovered video pointer: nullptr if there is nothing to discover
        Iterator::pointer discoverVideo(size_t index);

        /// @brief Download playlist info
        /// @throw std::runtime_error if internal error occurs
        /// @throw kb::Youtube::YoutubeError if YouTube playlist error occurs
        /// @throw kb::Youtube::LocalError if playlist is not supported
        void downloadInfo();

        /// @brief Check optional fields availability
        /// @throw std::runtime_error if internal error occurs
        /// @throw kb::Youtube::YoutubeError if YouTube error occurs
        void checkOptional() const;

    public:
        /// @brief Get playlist ID
        /// @return Playlist ID
        inline const std::string& id() const
        {
            return m_id;
        }

        /// @brief Get playlist view URL
        /// @return Playlist view URL
        inline std::string viewUrl() const
        {
            return ("https://www.youtube.com/playlist?list=" + m_id);
        }

        /// @brief Get playlist title
        /// @return Playlist title
        inline const std::string& title() const
        {
            return m_title;
        }

        /// @brief Get playlist author
        /// @return Playlist author
        inline const std::string& author() const
        {
            return m_author;
        }

        /// @brief Get playlist thumbnail URL
        /// @return Playlist thumbnail URL
        inline const std::string& thumbnailUrl() const
        {
            return m_thumbnailUrl;
        }

        /// @brief Get playlist video count
        /// @return Playlist video count
        inline int videoCount() const
        {
            return m_videoCount;
        }

        /// @brief Get playlist view count: optional info may be downloaded
        /// @throw std::runtime_error if internal error occurs
        /// @throw kb::Youtube::YoutubeError if YouTube error occurs
        /// @throw kb::Youtube::Error if playlist is not supported
        /// @return Playlist view count
        inline uint64_t viewCount() const
        {
            checkOptional();
            return m_viewCount;
        }

        /// @brief Check if playlist unavailable videos are hidden: optional info may be downloaded
        /// @throw std::runtime_error if internal error occurs
        /// @throw kb::Youtube::YoutubeError if YouTube error occurs
        /// @throw kb::Youtube::Error if playlist is not supported
        /// @return True if unavailable videos are hidden, false otherwise
        inline bool videosHidden() const
        {
            checkOptional();
            return m_videosHidden;
        }

        /// @brief Get playlist begin iterator
        /// @return Playlist begin iterator
        inline Iterator begin() const
        {
            Playlist* mutableThis = const_cast<Playlist*>(this);
            return Iterator(mutableThis);
        }

        /// @brief Get playlist iterator to last discovered video
        /// @return Playlist iterator to last discovered video
        inline Iterator last() const
        {
            if (m_videos.empty())
                return {};
            Playlist* mutableThis = const_cast<Playlist*>(this);
            return Iterator(mutableThis, m_videos.size() - 1);
        }

        /// @brief Get playlist end iterator
        /// @return Playlist end iterator
        inline Iterator end() const
        {
            return {};
        }
    };
}

} // namespace kb
