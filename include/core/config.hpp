#pragma once

#include <memory>
#include <string>

namespace kb {

namespace ConfigConst {
    constexpr const char* ConfigFile = "config.json";

    namespace Objects {
        constexpr const char* DiscordBotApiToken = "discord_bot_api_token";
        namespace Proxy {
            constexpr const char* Object = "proxy";
            constexpr const char* Enabled = "enabled";
            constexpr const char* Url = "url";
        }
    }

    namespace Defaults {
        constexpr const char* DiscordBotApiToken = "Enter Discord bot API token here";
        namespace Proxy {
            constexpr bool Enabled = false;
            constexpr const char* Url = "socks5h://localhost:8080";
        }
    }
}

class Config {
public:
    static void GenerateSampleFile();

private:
    static const std::unique_ptr<Config> Instance;
    std::string m_error;
    std::string m_discordBotApiToken;
    bool m_proxyEnabled;
    std::string m_proxyUrl;

private:
    Config();

public:
    static inline const std::string& GetError() {
        return Instance->m_error;
    }

    static inline const std::string& DiscordBotApiToken() {
        return Instance->m_discordBotApiToken;
    }
    
    static inline bool ProxyEnabled() {
        return Instance->m_proxyEnabled;
    }

    static inline const std::string& ProxyUrl() {
        return Instance->m_proxyUrl;
    }
};

} // namespace kb
