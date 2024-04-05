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
            LoginRequired,      // The item is private
            Unplayable,         // The item is blocked
            YoutubeError,       // Internal YouTube error
            PlaylistError,      // YouTube playlist error: playlist may be private
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
        /// @param itemId ID of item that created the error
        /// @param reason YouTube error reason
        /// @param subreason YouTube error subreason
        YoutubeError(Type type, const std::string& itemId, const std::string& reason, const std::string& subreason = "");

        /// @brief Get error type
        /// @return Error type
        inline Type type() const noexcept
        {
            return m_type;
        }

        /// @brief Get ID of item that created the error
        /// @return ID of item that created the error
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
            *   The item is a YouTube livestream.
            *   It is not supported and cannot be played.
            */
            LivestreamNotSupported,

            /*
            *   The item is a YouTube premiere.
            *   It is not supported and cannot be played.
            */
            PremiereNotSupported,

            /*
            *   The item is a YouTube #Shorts playlist. [Example: https://www.youtube.com/playlist?list=PLnN2bBxGARv7fRxsCcWaxvGE6sn5Ypp1H]
            *   It is not supported and cannot be played.
            */
            PlaylistNotSupported,

            /*
            *   Playlist only contains items that are:
            *       - YouTube livestreams;
            *       - YouTube premieres.
            *   They are not supported, hence the playlist cannot be played.
            */
            PlaylistItemsNotSupported,

            /*
            *   Playlist is empty.
            *   There is nothing to play.
            */
            EmptyPlaylist,

            /*
            *   None of the video's audio tracks are supported.
            *   Extractor expects an audio track that:
            *       - Is a WebM container;
            *       - Is encoded with Opus;
            *       - Contains 2 channels;
            *       - Has 48kHz sampling rate.
            */
            AudioNotSupported,

            /*
            *   Supported audio URL was found, but download was refused by YouTube.
            *   The signature decryption function may have failed.
            */
            CouldntDownload,
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
        /// @param itemId ID of item that created the error
        LocalError(Type type, const std::string& itemId);

        /// @brief Get error type
        /// @return Error type
        inline Type type() const noexcept
        {
            return m_type;
        }

        /// @brief Get ID of item that created the error
        /// @return ID of item that created the error
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
