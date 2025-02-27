#include "core/cache.hpp"
using namespace kb::CacheConst;

// STL modules
#include <fstream>
#include <filesystem>
#include <stdexcept>

// Library nlohmann::json
#include <nlohmann/json.hpp>

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

/*
*   std::make_unique() needs public constructor, but the Cache class uses singleton pattern.
*   This is why operator new is used instead.
*/
const std::unique_ptr<Cache> Cache::Instance(new Cache);

Cache::Cache()
{
    if (!std::filesystem::is_regular_file(CacheFile))
    {
        try
        {
            save();
            return;
        }
        catch (const std::runtime_error&)
        {
            m_error = "Couldn't open cache file";
            return;
        }
    }

    std::ifstream cacheFile(CacheFile);
    if (!cacheFile)
    {
        m_error = "Couldn't open cache file";
        return;
    }

    try
    {
        json cacheJson = json::parse(cacheFile);

        json youtubeAuthObject = cacheJson.at(Objects::YoutubeAuth::Object);
        m_youtubeAuth.authorized = youtubeAuthObject.at(Objects::YoutubeAuth::Authorized);
        m_youtubeAuth.accessToken = youtubeAuthObject.at(Objects::YoutubeAuth::AccessToken);
        m_youtubeAuth.accessTokenType = youtubeAuthObject.at(Objects::YoutubeAuth::AccessTokenType);
        m_youtubeAuth.expiresAt = youtubeAuthObject.at(Objects::YoutubeAuth::ExpiresAt);
        m_youtubeAuth.refreshToken = youtubeAuthObject.at(Objects::YoutubeAuth::RefreshToken);
    }
    catch (const json::exception&)
    {
        m_error = "Couldn't parse cache file JSON";
        return;
    }
}

void Cache::save()
{
    std::ofstream cacheFile(CacheFile, std::ios::trunc);
    if (!cacheFile)
        throw std::runtime_error("kb::Bot::Cache::save(): Couldn't open cache file");

    json youtubeAuthObject;
    youtubeAuthObject[Objects::YoutubeAuth::Authorized] = m_youtubeAuth.authorized;
    youtubeAuthObject[Objects::YoutubeAuth::AccessToken] = m_youtubeAuth.accessToken;
    youtubeAuthObject[Objects::YoutubeAuth::AccessTokenType] = m_youtubeAuth.accessTokenType;
    youtubeAuthObject[Objects::YoutubeAuth::ExpiresAt] = m_youtubeAuth.expiresAt;
    youtubeAuthObject[Objects::YoutubeAuth::RefreshToken] = m_youtubeAuth.refreshToken;

    json cacheJson;
    cacheJson[Objects::YoutubeAuth::Object] = youtubeAuthObject;
    cacheFile << cacheJson.dump(4) << '\n';
}

} // namespace kb
