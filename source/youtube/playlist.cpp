#include "youtube/playlist.hpp"

namespace kc {

Youtube::Playlist::Iterator::Iterator()
{
    invalidate();
}

Youtube::Playlist::Iterator::Iterator(Playlist* root)
    : m_root(root)
    , m_index(0)
{
    m_video = m_root->discoverVideo(0);
}

void Youtube::Playlist::Iterator::invalidate()
{
    m_root = nullptr;
    m_video = nullptr;
    m_index = -1;
}

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
            alertRendererObject = alertObject["alertWithButtonRenderer"];

        std::string alertMessage = Utility::ExtractString(alertRendererObject["text"]);
        if (alertRendererObject["type"] == "ERROR")
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

    m_author = Utility::ExtractString(playlistHeaderRendererObject["subtitle"]);
    boost::smatch matches;
    if (!boost::regex_match(m_author, matches, boost::regex(R"(^([\s\S]+) .+ Album$)")))
        throw std::runtime_error("kc::Youtube::Playlist::parseAuthor(): Couldn't parse author string: " + m_author);
    m_author = matches.str(1);
}

void Youtube::Playlist::parseThumbnailUrl(const json& playlistHeaderRendererObject)
{
    const json& heroPlaylistThumbnailRendererObject = playlistHeaderRendererObject["playlistHeaderBanner"]["heroPlaylistThumbnailRenderer"];
    if (heroPlaylistThumbnailRendererObject.contains("thumbnail"))
        m_thumbnailUrl = Utility::ExtractThumbnailUrl(heroPlaylistThumbnailRendererObject["thumbnail"]["thumbnails"]);
}

void Youtube::Playlist::parseVideos(const json& videoContentsObject)
{
    m_videos.reserve(m_videos.size() + videoContentsObject.size());
    for (const json& videoContentObject : videoContentsObject)
    {
        if (videoContentObject.contains("continuationItemRenderer"))
        {
            m_continuationToken = videoContentObject["continuationItemRenderer"]
                ["continuationEndpoint"]["continuationCommand"]["token"];
            break;
        }

        const json& playlistVideoRendererObject = videoContentObject["playlistVideoRenderer"];
        std::string videoTitle = Utility::ExtractString(playlistVideoRendererObject["title"]);

        /*
        *   Some playlist videos returned by YouTube API are private or deleted.
        *   They cannot be played and are not shown in YouTube client, so user is not even aware that they are there.
        *   Can be ignored.
        */
        if (videoTitle == "[Private video]" || videoTitle == "[Deleted video]")
            continue;

        m_videos.emplace_back(playlistVideoRendererObject);
    }
    m_videos.shrink_to_fit();

    /*
    *   We'll check if first page contains only unsupported videos.
    *   If it does, it is assumed that the whole playlist is of unsupported videos.
    */
    static bool firstPageChecked = false;
    if (!firstPageChecked)
    {
        firstPageChecked = true;

        bool allVideosUnsupported = true;
        for (const Video& video : m_videos)
        {
            if (video.type() == Video::Type::Normal)
            {
                allVideosUnsupported = false;
                break;
            }
        }
        if (allVideosUnsupported)
            throw LocalError(LocalError::Type::PlaylistOfUnplayableVideos, m_id);
    }

    /*
    *   YouTube API may return less videos than returned video count because unavailable videos may be hidden.
    *   Video count is set to real count when last page is reached.
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

Youtube::Playlist::Iterator::pointer Youtube::Playlist::discoverVideo(int index)
{
    if (!m_optionalKnown)
        downloadInfo();

    if (m_videos.size() - 1 >= index)
        return (m_videos.data() + index);

    if (m_continuationToken.empty())
        return nullptr;

    Curl::Response browseResponse = Client::Instance->requestApi(Client::Type::Web, "browse", { {"continuation", m_continuationToken} });
    if (browseResponse.code != 200)
        throw std::runtime_error("kc::Youtube::Playlist::discoverVideo(): Couldn't get YouTube API response");
    m_continuationToken.clear();

    try
    {
        json browseResponseJson = json::parse(browseResponse.data);
        parseVideos(browseResponseJson["onResponseReceivedActions"][0]
            ["appendContinuationItemsAction"]["continuationItems"]);
        return (m_videos.data() + index);
    }
    catch (const json::exception&)
    {
        throw std::runtime_error("kc::Youtube::Playlist::discoverVideo(): Couldn't parse YouTube API response JSON");
    }
}

Youtube::Playlist::Playlist(const std::string& idUrl)
    : m_id(idUrl)
{
    if (!boost::regex_match(m_id, boost::regex(ValidateId)))
    {
        boost::smatch matches;
        if (!boost::regex_search(m_id, matches, boost::regex(ExtractId)))
        {
            throw std::invalid_argument(fmt::format(
                "kc::Youtube::Playlist::Playlist(std::string): [idUrl]: \"{0}\": "
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
    m_id = playlistInfoObject["playlistId"];
    m_title = Utility::ExtractString(playlistInfoObject["title"]);
    m_author = Utility::ExtractString(playlistInfoObject["shortBylineText"]);
    m_thumbnailUrl = Utility::ExtractThumbnailUrl(playlistInfoObject["thumbnail"]["thumbnails"]);
    parseVideoCount(playlistInfoObject["videoCountShortText"]);
}

void Youtube::Playlist::downloadInfo()
{
    if (m_optionalKnown)
        return;

    Curl::Response browseResponse = Client::Instance->requestApi(Client::Type::Web, "browse", { {"browseId", "VL" + m_id} });
    if (browseResponse.code != 200)
        throw std::runtime_error("kc::Youtube::Playlist::downloadInfo(): Couldn't get YouTube API response");

    try
    {
        json browseResponseJson = json::parse(browseResponse.data);
        checkAlerts(browseResponseJson);

        const json& playlistHeaderRendererObject = browseResponseJson["header"]["playlistHeaderRenderer"];
        m_title = Utility::ExtractString(playlistHeaderRendererObject["title"]);
        parseAuthor(playlistHeaderRendererObject);
        parseThumbnailUrl(playlistHeaderRendererObject);
        parseVideoCount(playlistHeaderRendererObject["byline"][0]["playlistBylineRenderer"]["text"]);
        m_optionalKnown = true;
        m_viewCount = Utility::ExtractViewCount(playlistHeaderRendererObject["viewCountText"]);

        const json& contentsObject = browseResponseJson["contents"]["twoColumnBrowseResultsRenderer"]["tabs"][0]
            ["tabRenderer"]["content"]["sectionListRenderer"]["contents"][0]
            ["itemSectionRenderer"]["contents"][0];

        /*
        *   YouTube Shorts playlists are structured differently and can't be parsed.
        *   Example of such playlist: https://www.youtube.com/playlist?list=PLnN2bBxGARv7fRxsCcWaxvGE6sn5Ypp1H
        */
        if (contentsObject.contains("richGridRenderer"))
            throw LocalError(LocalError::Type::ShortsPlaylist, m_id);

        if (!contentsObject.contains("playlistVideoListRenderer"))
            throw LocalError(LocalError::Type::EmptyPlaylist, m_id);
        parseVideos(contentsObject["playlistVideoListRenderer"]["contents"]);
    }
    catch (const json::exception&)
    {
        throw std::runtime_error("kc::Youtube::Playlist::downloadInfo(): Couldn't parse YouTube API response JSON");
    }
}

void Youtube::Playlist::checkOptional() const
{
    /*
    *   It should be possible to call this function from getters, hence it is const.
    *   However optional fields getters may require the data to be downloaded, so it is okay to disable the const qualifier if needed.
    */
    if (!m_optionalKnown)
    {
        Playlist* mutableThis = const_cast<Playlist*>(this);
        mutableThis->downloadInfo();
    }
}

} // namespace kc
