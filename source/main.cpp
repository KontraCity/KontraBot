// STL modules
#include <iostream>

// Library Boost.Date_Time
#include <boost/date_time.hpp>

// Library nlohmann/json
#include <nlohmann/json.hpp>

// Custom modules
#include "common/utility.hpp"
#include "youtube/client.hpp"
#include "youtube/utility.hpp"
using namespace kc;

/* Namespace aliases and imports */
namespace dt = boost::gregorian;
namespace pt = boost::posix_time;

static void CommonUtilitiesTest()
{
    std::string string = "227,611,524";
    std::cout << "Original string: " << string << '\n';
    Utility::EraseCommas(string);
    std::cout << "Erased string: " << string << '\n';
    std::cout << '\n';

    std::cout << "Date: " << Utility::ToString(dt::day_clock::local_day()) << '\n';
    std::cout << "Time of day: " << Utility::ToString(pt::second_clock::local_time().time_of_day()) << '\n';
    std::cout << "Number: " << Utility::ToString(std::stoull(string)) << '\n';
    std::cout << '\n';

    std::cout << "Truncation:\n";
    string = "KontraBot";
    for (size_t length = string.length(); length != 0; --length)
        std::cout << length << ' ' << Utility::Truncate(string, length) << '\n';
}

static void YoutubeUtilitiesTest()
{
    Curl::Response playerResponse = Youtube::Client::Instance->requestApi(Youtube::Client::Type::Web, "player", { {"videoId", "Dn8vzTsnPps"} });
    json playerResponseJson = json::parse(playerResponse.data);
    
    std::cout << "Thumbnail URL: " << Youtube::Utility::ExtractThumbnailUrl(playerResponseJson["videoDetails"]["thumbnail"]["thumbnails"]) << '\n';
    std::cout << "String: " << Youtube::Utility::ExtractString(playerResponseJson["microformat"]["playerMicroformatRenderer"]["title"]) << '\n';
    std::cout << "View count: " << Youtube::Utility::ExtractViewCount(playerResponseJson["microformat"]["playerMicroformatRenderer"]["viewCount"].get<std::string>()) << '\n';
} 


int main()
{
    CommonUtilitiesTest();
    YoutubeUtilitiesTest();
}
