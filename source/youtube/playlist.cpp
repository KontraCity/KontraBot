#include "youtube/playlist.hpp"

namespace kc {

Youtube::Playlist::Iterator::Iterator()
    : m_root(nullptr)
    , m_video(nullptr)
    , m_index(-1)
{}

Youtube::Playlist::Iterator::Iterator(Playlist* root, size_t index)
    : m_root(root)
    , m_video(root->discoverVideo(index))
    , m_index(index)
{}

void Youtube::Playlist::checkAlerts(const json& browseResponseJson)
{
    if (!browseResponseJson.contains("alerts"))
        return;

    for (const json& alertObject : browseResponseJson["alerts"])
    {
        json alertRendererObject;
        if (alertObject.contains("alertRenderer"))
            alertRendererObject = alertObject["alertRenderer"];
        else
            alertRendererObject = alertObject.at("alertWithButtonRenderer");

        std::string alertMessage = Utility::ExtractString(alertRendererObject.at("text"));
        if (alertRendererObject.at("type") == "ERROR")
            throw YoutubeError(YoutubeError::Type::PlaylistError, m_id, alertMessage);
        m_videosHidden = boost::regex_search(alertMessage, boost::regex(R"(navailable.+video.+hidden)"));
    }
}

void Youtube::Playlist::parseAuthor(const json& playlistHeaderRendererObject)
{
    if (playlistHeaderRendererObject.contains("ownerText"))
    {
        m_author = Utility::ExtractString(playlistHeaderRendererObject["ownerText"]);
        return;
    }

    m_author = Utility::ExtractString(playlistHeaderRendererObject.at("subtitle"));
    boost::smatch matches;
    if (!boost::regex_match(m_author, matches, boost::regex(R"(^([\s\S]+) .+ Album$)")))
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Playlist::parseAuthor(): "
            "Couldn't extract author from author string [playlist: \"{}\", author string: \"{}\"]",
            m_id, m_author
        ));
    }
    m_author = matches.str(1);
}

void Youtube::Playlist::parseThumbnailUrlHeaderRenderer(const json& playlistHeaderRendererObject)
{
    const json& heroPlaylistThumbnailRendererObject = playlistHeaderRendererObject.at("playlistHeaderBanner").at("heroPlaylistThumbnailRenderer");
    if (heroPlaylistThumbnailRendererObject.contains("thumbnail"))
        m_thumbnailUrl = Utility::ExtractThumbnailUrl(heroPlaylistThumbnailRendererObject["thumbnail"].at("thumbnails"));
}

void Youtube::Playlist::parseThumbnailUrlSidebarRenderer(const json& playlistSidebarPrimaryInfoRendererObject)
{
    const json& playlistVideoThumbnailRendererObject = playlistSidebarPrimaryInfoRendererObject.at("thumbnailRenderer").at("playlistVideoThumbnailRenderer");
    if (playlistVideoThumbnailRendererObject.contains("thumbnail"))
        m_thumbnailUrl = Utility::ExtractThumbnailUrl(playlistVideoThumbnailRendererObject["thumbnail"].at("thumbnails"));
}

void Youtube::Playlist::parseVideos(const json& videoContentsObject)
{
    m_videos.reserve(m_videos.size() + videoContentsObject.size());
    for (const json& videoContentObject : videoContentsObject)
    {
        if (videoContentObject.contains("continuationItemRenderer"))
        {
            m_continuationToken = videoContentObject.at("continuationItemRenderer")
                .at("continuationEndpoint").at("continuationCommand").at("token");
            break;
        }

        const json& playlistVideoRendererObject = videoContentObject.at("playlistVideoRenderer");
        std::string videoTitle = Utility::ExtractString(playlistVideoRendererObject.at("title"));
        if (videoTitle == "[Private video]" || videoTitle == "[Deleted video]")
        {
            /*
            *   Some playlist videos returned by YouTube API are private or deleted.
            *   They cannot be played and are not shown in YouTube client, so user is not even aware that they are there.
            *   Can be ignored.
            */
            continue;
        }
        m_videos.emplace_back(playlistVideoRendererObject);
    }
    m_videos.shrink_to_fit();

    /*
    *   We'll check if first page contains only unsupported items.
    *   If it does, it is assumed that the whole playlist is of unsupported items.
    */
    static bool firstPage = true;
    if (firstPage)
    {
        firstPage = false;
        bool allVideosNotSupported = std::all_of(
            m_videos.begin(),
            m_videos.end(),
            [](const Youtube::Video& video) { return video.type() != Video::Type::Normal; }
        );
        if (allVideosNotSupported)
            throw LocalError(LocalError::Type::PlaylistItemsNotSupported, m_id);
    }

    /*
    *   YouTube API may return less videos than promised video count because unavailable videos may be hidden.
    *   Video count must be set to real count when last page is reached.
    */
    if (m_continuationToken.empty())
        m_videoCount = static_cast<int>(m_videos.size());
}

void Youtube::Playlist::parseVideoCount(const json& videoCountObject)
{
    std::string videoCountString = Utility::ExtractString(videoCountObject);
    videoCountString.erase(
        std::remove_if(
            videoCountString.begin(),
            videoCountString.end(),
            [](char character) { return (character == ','); }
        ),
        videoCountString.end()
    );
    m_videoCount = (videoCountString == "No videos" ? 0 : std::stoi(videoCountString));
}

Youtube::Playlist::Iterator::pointer Youtube::Playlist::discoverVideo(size_t index)
{
    if (!m_optionalKnown)
        downloadInfo();
    if (m_videos.size() - 1 >= index)
        return (m_videos.data() + index);
    if (m_continuationToken.empty())
        return nullptr;

    Curl::Response browseResponse = Client::Instance->requestApi(Client::Type::Web, "browse", { {"continuation", m_continuationToken} });
    if (browseResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Playlist::discoverVideo(): "
            "Couldn't get API response [playlist: \"{}\", client: \"web\", response code: {}]",
            m_id, browseResponse.code
        ));
    }
    m_continuationToken.clear();

    try
    {
        json browseResponseJson = json::parse(browseResponse.data);
        parseVideos(browseResponseJson.at("onResponseReceivedActions").at(0)
            .at("appendContinuationItemsAction").at("continuationItems"));
        return (m_videos.data() + index);
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Playlist::discoverVideo(): "
            "Couldn't parse API JSON response [playlist: \"{}\", client: \"web\", id: {}]",
            m_id, error.id
        ));
    }
}

void Youtube::Playlist::downloadInfo()
{
    if (m_optionalKnown)
        return;

    Curl::Response browseResponse = Client::Instance->requestApi(Client::Type::Web, "browse", { {"browseId", "VL" + m_id} });
    if (browseResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Playlist::downloadInfo(): "
            "Couldn't get API response [playlist: \"{}\", client: \"web\", response code: {}]",
            m_id, browseResponse.code
        ));
    }

    try
    {
        json browseResponseJson = json::parse(browseResponse.data);
        checkAlerts(browseResponseJson);

        if (browseResponseJson.at("header").contains("playlistHeaderRenderer"))
        {
            const json& playlistHeaderRendererObject = browseResponseJson["header"]["playlistHeaderRenderer"];
            m_title = Utility::ExtractString(playlistHeaderRendererObject.at("title"));
            parseAuthor(playlistHeaderRendererObject);
            parseThumbnailUrlHeaderRenderer(playlistHeaderRendererObject);
            parseVideoCount(playlistHeaderRendererObject.at("byline").at(0).at("playlistBylineRenderer").at("text"));
            m_optionalKnown = true;
            m_viewCount = Utility::ExtractViewCount(playlistHeaderRendererObject.at("viewCountText"));
        }
        else
        {
            const json& playlistSidebarPrimaryInfoRendererObject = browseResponseJson.at("sidebar").at("playlistSidebarRenderer").at("items").at(0).at("playlistSidebarPrimaryInfoRenderer");
            const json& playlistSidebarSecondaryInfoRendererObject = browseResponseJson.at("sidebar").at("playlistSidebarRenderer").at("items").at(1).at("playlistSidebarSecondaryInfoRenderer");
            m_title = Utility::ExtractString(playlistSidebarPrimaryInfoRendererObject.at("title"));
            m_author = Utility::ExtractString(playlistSidebarSecondaryInfoRendererObject.at("videoOwner").at("videoOwnerRenderer").at("title"));
            parseThumbnailUrlSidebarRenderer(playlistSidebarPrimaryInfoRendererObject);
            parseVideoCount(playlistSidebarPrimaryInfoRendererObject.at("stats").at(0));
            m_optionalKnown = true;
            m_viewCount = Utility::ExtractViewCount(playlistSidebarPrimaryInfoRendererObject.at("stats").at(1));
        }

        const json& contentsObject = browseResponseJson.at("contents").at("twoColumnBrowseResultsRenderer").at("tabs").at(0)
            .at("tabRenderer").at("content").at("sectionListRenderer").at("contents").at(0)
            .at("itemSectionRenderer").at("contents").at(0);

        /*
        *   YouTube #Shorts playlists are structured differently and can't be parsed.
        *   Example of such playlist: https://www.youtube.com/playlist?list=PLnN2bBxGARv7fRxsCcWaxvGE6sn5Ypp1H
        */
        if (contentsObject.contains("richGridRenderer"))
            throw LocalError(LocalError::Type::PlaylistNotSupported, m_id);
        if (!contentsObject.contains("playlistVideoListRenderer"))
            throw LocalError(LocalError::Type::EmptyPlaylist, m_id);
        parseVideos(contentsObject["playlistVideoListRenderer"].at("contents"));
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Playlist::downloadInfo(): "
            "Couldn't parse API JSON response [playlist: \"{}\", client: \"web\", id: {}]",
            m_id, error.id
        ));
    }
}

void Youtube::Playlist::checkOptional() const
{
    if (m_optionalKnown)
        return;

    /*
    *   It should be possible to call this function from getters, hence it is const.
    *   However optional fields may require the data to be downloaded, so it is okay to disable const qualifier.
    */
    Playlist* mutableThis = const_cast<Playlist*>(this);
    mutableThis->downloadInfo();
}

Youtube::Playlist::Playlist(const std::string& idUrl)
    : m_id(idUrl)
{
    if (!boost::regex_match(m_id, boost::regex(PlaylistConst::ValidateId)))
    {
        boost::smatch matches;
        if (!boost::regex_search(m_id, matches, boost::regex(PlaylistConst::ExtractId)))
        {
            throw std::invalid_argument(fmt::format(
                "kc::Youtube::Playlist::Playlist(): [idUrl]: \"{}\": "
                "Not a valid playlist ID or view URL",
                m_id
            ));
        }
        m_id = matches[1].str();
    }

    downloadInfo();
}

Youtube::Playlist::Playlist(const json& playlistInfoObject)
{
    m_id = playlistInfoObject.at("playlistId");
    m_title = Utility::ExtractString(playlistInfoObject.at("title"));
    m_author = Utility::ExtractString(playlistInfoObject.at("shortBylineText"));
    m_thumbnailUrl = Utility::ExtractThumbnailUrl(playlistInfoObject.at("thumbnail").at("thumbnails"));
    parseVideoCount(playlistInfoObject.at("videoCountShortText"));
}

} // namespace kc
