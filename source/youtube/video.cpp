#include "youtube/video.hpp"

// STL modules
#include <sstream>
#include <algorithm>
#include <stdexcept>

// Boost libraries
#include <boost/regex.hpp>

// Custom modules
#include "common/utility.hpp"
#include "youtube/client.hpp"
#include "youtube/error.hpp"
#include "youtube/utility.hpp"

namespace kb {

Youtube::Video::Video(const std::string& idUrl)
    : m_id(idUrl)
{
    if (!boost::regex_match(m_id, boost::regex(VideoConst::ValidateId)))
    {
        boost::smatch matches;
        if (!boost::regex_search(m_id, matches, boost::regex(VideoConst::ExtractId)))
        {
            throw std::invalid_argument(fmt::format(
                "kb::Youtube::Video::Video(): [idUrl]: \"{}\": "
                "Not a valid video ID or watch URL",
                m_id
            ));
        }
        m_id = matches.str(1);
    }

    downloadInfo();
}

Youtube::Video::Video(const json& videoInfoObject)
{
    m_id = videoInfoObject.at("videoId");
    m_title = Utility::ExtractString(videoInfoObject.at("title"));
    m_author = Utility::ExtractString(videoInfoObject.at("shortBylineText"));
    m_thumbnailUrl = Utility::ExtractThumbnailUrl(videoInfoObject.at("thumbnail").at("thumbnails"));

    if (videoInfoObject.contains("upcomingEventData"))
    {
        m_type = Type::Premiere;
        return;
    }

    if (!parseDuration(videoInfoObject))
        m_type = Type::Livestream;
    parseViewCount(videoInfoObject);
}

bool Youtube::Video::parseDuration(const json& videoInfoObject)
{
    if (videoInfoObject.contains("lengthSeconds"))
    {
        m_duration = pt::time_duration(0, 0, std::stoull(videoInfoObject["lengthSeconds"].get<std::string>()));
        return true;
    }

    if (!videoInfoObject.contains("lengthText"))
        return false;

    std::string durationString = Utility::ExtractString(videoInfoObject["lengthText"]);
    boost::smatch matches;
    if (!boost::regex_match(durationString, matches, boost::regex(R"((?:(\d{1,3}):)?(\d{1,2}):(\d{2}))")))
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Video::parseDuration(): "
            "Couldn't extract duration components from duration string [video: \"{}\", duration string: \"{}\"]",
            m_id, durationString
        ));
    }

    std::string hours = matches.str(1);
    m_duration = pt::time_duration(
        (hours.empty() ? 0 : std::stoi(hours)),
        std::stoi(matches.str(2)),
        std::stoi(matches.str(3))
    );
    return true;
}

void Youtube::Video::parseViewCount(const json& videoInfoObject)
{
    if (videoInfoObject.contains("viewCountText"))
    {
        m_viewCount = Utility::ExtractViewCount(videoInfoObject["viewCountText"]);
        return;
    }

    std::string accessibilityString = videoInfoObject.at("title").at("accessibility").at("accessibilityData").at("label");
    boost::smatch matches;
    if (!boost::regex_search(accessibilityString, matches, boost::regex(R"(([\d,]+ view|No views))")))
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Video::parseViewCount(): "
            "Couldn't extract view count string from accessibility string [video: \"{}\", accessibility string: \"{}\"]",
            m_id, accessibilityString
        ));
    }
    m_viewCount = Utility::ExtractViewCount(matches.str(1));
}

void Youtube::Video::parseUploadDate(std::string uploadDateString)
{
    /*
    *   Almost always YouTube API returns upload date string as something like "2021-09-22".
    *   Howewer, sometimes the string may be returned as "2021-09-22T06:47:53-07:00".
    *   Needless part of string must be removed to ensure that dt::from_simple_string() doesn't fail.
    */
    size_t timeStartLocation = uploadDateString.find("T");
    if (timeStartLocation != std::string::npos)
        uploadDateString.erase(uploadDateString.begin() + timeStartLocation, uploadDateString.end());
    m_uploadDate = dt::from_simple_string(uploadDateString);
}

void Youtube::Video::parseChapters(const std::string& description)
{
    std::stringstream descriptionStream(description);
    while (!descriptionStream.eof())
    {
        std::string line;
        std::getline(descriptionStream, line);
        if (line.empty())
            continue;

        boost::smatch matches;
        if (!boost::regex_search(line, matches, boost::regex(R"((?:(\d+?):)?(\d+?):(\d+))")))
            continue;

        std::string hours = matches.str(1);
        pt::time_duration timestamp(
            (hours.empty() ? 0 : std::stoi(hours)),
            std::stoi(matches.str(2)),
            std::stoi(matches.str(3))
        );

        line.replace(matches.position(), matches.str(0).length(), "");
        if (boost::regex_search(line, matches, boost::regex(R"([- ]*(?:\d+?:)?\d+?:\d+)")))
            line.replace(matches.position(), matches.str(0).length(), "");
        if (boost::regex_search(line, matches, boost::regex(R"(^(?:[-\|: ]+))")))
            line.replace(matches.position(), matches.str(0).length(), "");
        if (boost::regex_search(line, matches, boost::regex(R"((?:[-\|: ]+)$)")))
            line.replace(matches.position(), matches.str(0).length(), "");
        m_chapters.push_back({ m_chapters.size() + 1, kb::Utility::Truncate(line, 100), timestamp });
    }

    if (m_chapters.empty())
        return;

    if (m_chapters[0].timestamp.total_seconds() != 0 || !std::all_of(m_chapters.begin(), m_chapters.end(), [](const Chapter& chapter) { return !chapter.name.empty(); }))
    {
        m_chapters.clear();
        return;
    }

    for (size_t index = 0, size = m_chapters.size(); index < size - 1; ++index)
        m_chapters[index].duration = m_chapters[index + 1].timestamp - m_chapters[index].timestamp;
    m_chapters[m_chapters.size() - 1].duration = m_duration - m_chapters[m_chapters.size() - 1].timestamp;
}

void Youtube::Video::checkPlayabilityStatus(const json& playabilityStatusObject)
{
    std::string playabilityStatusString = playabilityStatusObject.at("status");
    if (playabilityStatusString == "OK")
        return;

    if (playabilityStatusString == "CONTENT_CHECK_REQUIRED")
    {
        /*
        *   Video may contain sensitive content for some viewers.
        *   This doesn't affect data extraction and can be ignored.
        */
        return;
    }

    if (playabilityStatusString == "LIVE_STREAM_OFFLINE")
    {
        /*
        *   Video is a premiere and is not available yet.
        *   Although premieres are not supported, they are not handled as exceptions.
        *   Can be ignored.
        */
        return;
    }

    YoutubeError::Type type;
    if (playabilityStatusString == "LOGIN_REQUIRED")
        type = YoutubeError::Type::LoginRequired;
    else if (playabilityStatusString == "UNPLAYABLE")
        type = YoutubeError::Type::Unplayable;
    else if (playabilityStatusString == "ERROR")
        type = YoutubeError::Type::YoutubeError;
    else
        throw YoutubeError(YoutubeError::Type::Unknown, m_id, playabilityStatusString);

    if (!playabilityStatusObject.at("errorScreen").contains("playerErrorMessageRenderer"))
        throw YoutubeError(type, m_id, playabilityStatusObject.at("reason").get<std::string>());

    const json& playerErrorMessageRendererObject = playabilityStatusObject["errorScreen"]["playerErrorMessageRenderer"];
    std::string reason = Utility::ExtractString(playerErrorMessageRendererObject.at("reason"));
    if (reason == "Sign in to confirm your age")
    {
        /*
        *   Video is age-restricted.
        *   TV embedded player bypasses the restriction. Error can be ignored.
        */
        return;
    }

    if (!playerErrorMessageRendererObject.contains("subreason"))
        throw YoutubeError(type, m_id, reason);
    throw YoutubeError(type, m_id, reason, Utility::ExtractString(playerErrorMessageRendererObject["subreason"]));
}

void Youtube::Video::downloadInfo()
{
    if (m_optionalKnown)
        return;

    Curl::Response playerResponse = Client::Instance->requestApi(Client::Type::Web, "player", { {"videoId", m_id} });
    if (playerResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Video::downloadInfo(): "
            "Couldn't get API response [video: \"{}\", client: \"web\", response code: {}]",
            m_id, playerResponse.code
        ));
    }

    try
    {
        json playerResponseJson = json::parse(playerResponse.data);
        checkPlayabilityStatus(playerResponseJson.at("playabilityStatus"));

        const json& videoDetailsObject = playerResponseJson.at("videoDetails");
        m_title = videoDetailsObject.at("title");
        m_author = videoDetailsObject.at("author");
        m_thumbnailUrl = Utility::ExtractThumbnailUrl(videoDetailsObject.at("thumbnail").at("thumbnails"));

        json playerMicroformatRendererObject = playerResponseJson.at("microformat").at("playerMicroformatRenderer");
        m_optionalKnown = true;
        m_category = playerMicroformatRendererObject.at("category");
        parseUploadDate(playerMicroformatRendererObject.at("uploadDate"));

        if (videoDetailsObject.contains("isUpcoming") && videoDetailsObject["isUpcoming"])
        {
            m_type = Type::Premiere;
            return;
        }

        if (videoDetailsObject.contains("isLive") && videoDetailsObject["isLive"])
            m_type = Type::Livestream;

        m_duration = pt::time_duration(0, 0, std::stoull(videoDetailsObject.at("lengthSeconds").get<std::string>()));
        m_viewCount = std::stoull(videoDetailsObject.at("viewCount").get<std::string>());
        if (playerMicroformatRendererObject.contains("description"))
            parseChapters(Utility::ExtractString(playerMicroformatRendererObject["description"]));
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Video::downloadInfo(): "
            "Couldn't parse API JSON response [video: \"{}\", client: \"web\", id: {}]",
            m_id, error.id
        ));
    }
}

void Youtube::Video::checkOptional() const
{
    if (m_optionalKnown)
        return;

    /*
    *   It should be possible to call this function from getters, hence it is const.
    *   However optional fields may require the data to be downloaded, so it is okay to disable const qualifier.
    */
    Video* mutableThis = const_cast<Video*>(this);
    mutableThis->downloadInfo();
}

} // namespace kb
