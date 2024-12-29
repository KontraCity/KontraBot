#pragma once

// STL modules
#include <memory>
#include <mutex>
#include <string>

namespace kc {

namespace CacheConst
{
    // Bot cache JSON file name
    constexpr const char* CacheFile = "cache.json";

    namespace Objects
    {
        namespace YoutubeAuth
        {
            constexpr const char* Object = "youtube_auth";
            constexpr const char* Authorized = "authorized";
            constexpr const char* AccessToken = "access_token";
            constexpr const char* AccessTokenType = "access_token_type";
            constexpr const char* ExpiresAt = "expires_at";
            constexpr const char* RefreshToken = "refresh_token";
        }
    }
}

class Cache
{
public:
    struct YoutubeAuth
    {
        bool authorized = false;
        std::string accessToken;
        std::string accessTokenType;
        int expiresAt = 0;
        std::string refreshToken;
    };

public:
    // Singleton instance
    static const std::unique_ptr<Cache> Instance;

private:
    std::mutex m_mutex;
    std::string m_error;
    YoutubeAuth m_youtubeAuth;

private:
    /// @brief Create or read and parse cache file
    Cache();

    /// @brief Save cache file
    /// @throw std::runtime_error if internal error occurs
    void save();

public:
    /// @brief Get cache file error message
    /// @return Cache file error message (empty if no error)
    inline const std::string& error() const
    {
        return m_error;
    }

    /// @brief Get YouTube authorization info
    /// @return YouTube authorization info
    inline YoutubeAuth getYoutubeAuth()
    {
        std::lock_guard lock(m_mutex);
        return m_youtubeAuth;
    }

    /// @brief Set YouTube authorization info
    /// @param youtubeAuth The authorization info to set
    /// @throw std::runtime_error if internal error occurs
    inline void setYoutubeAuth(const YoutubeAuth& youtubeAuth)
    {
        std::lock_guard lock(m_mutex);
        m_youtubeAuth = youtubeAuth;
        save();
    }
};

} // namespace kc
