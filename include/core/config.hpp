#pragma once

// STL modules
#include <memory>
#include <string>

namespace kb {

namespace ConfigConst
{
    // Bot configuration JSON file name
    constexpr const char* ConfigFile = "config.json";

    namespace Objects
    {
        constexpr const char* DiscordBotApiToken = "discord_bot_api_token";
        constexpr const char* YoutubeAuthEnabled = "youtube_auth_enabled";

        namespace Proxy
        {
            constexpr const char* Object = "proxy";
            constexpr const char* Enabled = "enabled";
            constexpr const char* Url = "url";
        }
    }

    namespace Defaults
    {
        constexpr const char* DiscordBotApiToken = "Enter Discord bot API token here";
        constexpr bool YoutubeAuthEnabled = false;

        namespace Proxy
        {
            constexpr bool Enabled = false;
            constexpr const char* Url = "socks5h://localhost:8080";
        }
    }
}

class Config
{
public:
    // Singleton instance
    static const std::unique_ptr<Config> Instance;

public:
    /// @brief Generate sample configuration file for user to fill out
    /// @throw std::runtime_error if file couldn't be created
    static void GenerateSampleFile();

private:
    std::string m_error;

    // Common configuration
    std::string m_discordBotApiToken;
    bool m_youtubeAuthEnabled = ConfigConst::Defaults::YoutubeAuthEnabled;

    // Proxy configuration
    bool m_proxyEnabled = ConfigConst::Defaults::Proxy::Enabled;
    std::string m_proxyUrl;

private:
    /// @brief Read and parse configuration file
    Config();

public:
    /// @brief Get configuration file error message
    /// @return Configuration file error message (empty if no error)
    inline const std::string& error() const
    {
        return m_error;
    }

    /// @brief Get Discord bot API token
    /// @return Discord bot API token
    inline const std::string& discordBotApiToken() const
    {
        return m_discordBotApiToken;
    }

    /// @brief Check if youtube authentication is enabled
    /// @return True if youtube authentication is enabled
    inline bool youtubeAuthEnabled() const
    {
        return m_youtubeAuthEnabled;
    }

    /// @brief Check if proxy is enabled
    /// @return True if proxy is enabled
    inline bool proxyEnabled() const
    {
        return m_proxyEnabled;
    }

    /// @brief Get proxy URL
    /// @return Proxy URL
    inline const std::string& proxyUrl() const
    {
        return m_proxyUrl;
    }
};

} // namespace kb
