#pragma once

// STL modules
#include <string>
#include <stdexcept>

// Library {fmt}
#include <fmt/format.h>

namespace kc {

namespace Youtube
{
    class YoutubeError : public std::exception
    {
    public:
        enum class Type
        {
            LoginRequired,      // Probably the item is private
            Unplayable,         // Probably the item is blocked
            YoutubeError,       // Internal YouTube error
            PlaylistError,      // Internal YouTube playlist error
            Unknown,            // Unknown YouTube error
        };

    private:
        Type m_type;
        std::string m_itemId;
        std::string m_reason;
        std::string m_subreason;
        std::string m_what;

    private:
        /// @brief Convert error type to error name
        /// @param type Error type
        /// @return Error name
        static const char* TypeToName(Type type);

    public:
        /// @brief Create YouTube error
        /// @param type Error type
        /// @param itemId ID of item error is related to
        /// @param reason YouTube error reason
        /// @param subreason YouTube error subreason
        YoutubeError(Type type, const std::string& itemId, const std::string& reason, const std::string& subreason = "");

        /// @brief Get error type
        /// @return Error type
        inline Type type() const noexcept
        {
            return m_type;
        }

        /// @brief Get ID of item error is related to
        /// @return ID of item error is related to
        inline const std::string& itemId() const noexcept
        {
            return m_itemId;
        }

        /// @brief Get YouTube error reason
        /// @return YouTube error reason
        inline const std::string& reason() const noexcept
        {
            return m_reason;
        }

        /// @brief Get YouTube error subreason
        /// @return YouTube error subreason
        inline const std::string& subreason() const noexcept
        {
            return m_subreason;
        }

        /// @brief Get brief error description
        /// @return Brief error description
        inline const char* what() const noexcept
        {
            return m_what.c_str();
        }
    };

    class LocalError : public std::exception
    {
    public:
        enum class Type
        {
            /*
            *   Playlist only contains videos that are:
            *       - YouTube livestreams;
            *       - YouTube premieres.
            *   They are unplayable.
            */
            PlaylistOfUnplayableVideos,

            /*
            *   YouTube Shorts playlists are structured differently and can't be parsed.
            *   Example of such playlist: https://www.youtube.com/playlist?list=PLnN2bBxGARv7fRxsCcWaxvGE6sn5Ypp1H
            */
            ShortsPlaylist,

            /*
            *   Playlist is empty.
            *   There is nothing to play.
            */
            EmptyPlaylist,

            // YouTube API produced an unknown error
            UnknownYoutubeError,

            /*
            *   Couldn't find audio format that is:
            *       - A WebM container;
            *       - Opus encoded;
            *       - Contains 2 channels;
            *       - Has 48kHz sampling rate.
            */
            FormatNotFound,

            /*
            *   An audio format was found, but download request failed.
            *   Signature extract function may have failed.
            */
            DownloadError,
        };

    private:
        Type m_type;
        std::string m_itemId;
        std::string m_what;

    private:
        /// @brief Convert error type to error name
        /// @param type Error type
        /// @return Error name
        static const char* TypeToName(Type type);

    public:
        /// @brief Create error
        /// @param type Error type
        /// @param itemId ID of item error is related to
        LocalError(Type type, const std::string& itemId);

        /// @brief Get error type
        /// @return Error type
        inline Type type() const noexcept
        {
            return m_type;
        }

        /// @brief Get ID of item error is related to
        /// @return ID of item error is related to
        inline const std::string& itemId() const noexcept
        {
            return m_itemId;
        }

        /// @brief Get brief error description
        /// @return Brief error description
        inline const char* what() const noexcept
        {
            return m_what.c_str();
        }
    };
}

} // namespace kc
