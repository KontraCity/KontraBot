#include "youtube/error.hpp"

namespace kc {

const char* Youtube::YoutubeError::TypeToName(Type type)
{
    switch (type)
    {
        case Type::LoginRequired:
            return "LOGIN_REQUIRED";
        case Type::Unplayable:
            return "UNPLAYABLE";
        case Type::YoutubeError:
            return "ERROR";
        case Type::PlaylistError:
            return "Playlist error";
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

const char* Youtube::LocalError::TypeToReason(Type type)
{
    switch (type)
    {
            return "Premiere is not supported";
        case Type::PlaylistNotSupported:
            return "Playlist is not supported";
        case Type::PlaylistItemsNotSupported:
            return "Playlist items are not supported";
        case Type::EmptyPlaylist:
            return "Playlist is empty";
        case Type::AudioNotSupported:
            return "None of the video's audio tracks are supported";
        case Type::CouldntDownload:
            return "Audio URL was found, but download was refused by YouTube";
        default:
            return "Unknown error";
    }
}

Youtube::LocalError::LocalError(Type type, const std::string& itemId)
    : m_type(type)
    , m_itemId(itemId)
    , m_what(fmt::format("Item \"{}\": {}", itemId, TypeToReason(type)))
{}

} // namespace kc
