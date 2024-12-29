#pragma once

// STL modules
#include <memory>
#include <string>

namespace kc {

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
            constexpr const char* Host = "host";
            constexpr const char* Port = "port";

            namespace Auth
            {
                constexpr const char* Object = "auth";
                constexpr const char* Required = "required";
                constexpr const char* User = "user";
                constexpr const char* Password = "password";
            }
        }
    }

    namespace Defaults
    {
        constexpr const char* DiscordBotApiToken = "Enter Discord bot API token here";
        constexpr bool YoutubeAuthEnabled = false;

        namespace Proxy
        {
            constexpr bool Enabled = false;
            constexpr const char* Host = "0.0.0.0";
            constexpr uint16_t Port = 8080;

            namespace Auth
            {
                constexpr bool Required = false;
                constexpr const char* User = "user";
                constexpr const char* Password = "password";
            }
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

    std::string m_discordBotApiToken;
    bool m_youtubeAuthEnabled;

    bool m_proxyEnabled;
    std::string m_proxyHost;
    uint16_t m_proxyPort;

    bool m_proxyAuthRequired;
    std::string m_proxyAuthUser;
    std::string m_proxyAuthPassword;

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

    /// @brief Get proxy host
    /// @return Proxy host
    inline const std::string& proxyHost() const
    {
        return m_proxyHost;
    }

    /// @brief Get proxy port
    /// @return Proxy port
    inline uint16_t proxyPort() const
    {
        return m_proxyPort;
    }

    /// @brief Check if proxy authentication is required
    /// @return True if proxy authentication is enabled
    inline bool proxyAuthRequired() const
    {
        return m_proxyAuthRequired;
    }

    /// @brief Get proxy authentication user
    /// @return Proxy authentication user
    inline const std::string& proxyAuthUser() const
    {
        return m_proxyAuthUser;
    }

    /// @brief Get proxy authentication password
    /// @return Proxy authentication password
    inline const std::string& proxyAuthPassword() const
    {
        return m_proxyAuthPassword;
    }
};

} // namespace kc
