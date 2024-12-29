#include "youtube/error.hpp"

// Library {fmt}
#include <fmt/format.h>

namespace kb {

/// @brief Convert error type to error name
/// @param type Error type
/// @return Error name
static const char* TypeToName(Youtube::YoutubeError::Type type)
{
    switch (type)
    {
        case Youtube::YoutubeError::Type::LoginRequired:
            return "LOGIN_REQUIRED";
        case Youtube::YoutubeError::Type::Unplayable:
            return "UNPLAYABLE";
        case Youtube::YoutubeError::Type::YoutubeError:
            return "ERROR";
        case Youtube::YoutubeError::Type::PlaylistError:
            return "Playlist error";
        default:
            return "Unknown error";
    }
}

/// @brief Convert error type to error reason
/// @param type Error type
/// @return Error reason
static const char* TypeToReason(Youtube::LocalError::Type type)
{
    switch (type)
    {
        case Youtube::LocalError::Type::PlaylistNotSupported:
            return "Playlist is not supported";
        case Youtube::LocalError::Type::PlaylistItemsNotSupported:
            return "Playlist items are not supported";
        case Youtube::LocalError::Type::EmptyPlaylist:
            return "Playlist is empty";
        case Youtube::LocalError::Type::AudioNotSupported:
            return "None of the video's audio tracks are supported";
        case Youtube::LocalError::Type::CouldntDownload:
            return "Audio URL was found, but download was refused by YouTube";
        default:
            return "Unknown error";
    }
}

Youtube::YoutubeError::YoutubeError(Type type, const std::string& itemId, const std::string& reason, const std::string& subreason)
    : m_type(type)
    , m_itemId(itemId)
    , m_reason(reason)
    , m_subreason(subreason)
{
    if (subreason.empty())
        m_what = fmt::format("Item \"{}\": {}: {}", itemId, TypeToName(type), reason);
    else
        m_what = fmt::format("Item \"{}\": {}: {}; {}", itemId, TypeToName(type), reason, subreason);
}

Youtube::LocalError::LocalError(Type type, const std::string& itemId)
    : m_type(type)
    , m_itemId(itemId)
    , m_what(fmt::format("Item \"{}\": {}", itemId, TypeToReason(type)))
{}

} // namespace kb
