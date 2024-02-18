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

const char* Youtube::LocalError::TypeToName(Type type)
{
    switch (type)
    {
        case Type::PlaylistOfUnplayableVideos:
            return "All playlist videos are unplayable";
        case Type::ShortsPlaylist:
            return "YouTube Shorts playlists can't be played";
        case Type::EmptyPlaylist:
            return "Playlist is empty";
        case Type::UnknownYoutubeError:
            return "Unknown YouTube error occured";
        case Type::FormatNotFound:
            return "Couldn't find suitable audio format";
        case Type::DownloadError:
            return "Couldn't initiate audio download";
        default:
            return "Unknown error";
    }
}

Youtube::LocalError::LocalError(Type type, const std::string& itemId)
    : m_type(type)
    , m_itemId(itemId)
    , m_what(TypeToName(type))
{}

} // namespace kc
