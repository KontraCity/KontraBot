#include "core/config.hpp"

#include <nlohmann/json.hpp>
using nlohmann::json;

#include "core/error.hpp"
#include "core/io.hpp"

namespace kb {

namespace Objects {
    constexpr const char* DiscordBotApiToken = "discord_bot_api_token";
    constexpr const char* YoutubeAuthEnabled = "youtube_auth_enabled";

    namespace Proxy {
        constexpr const char* Object = "proxy";
        constexpr const char* Enabled = "enabled";
        constexpr const char* Url = "url";
    }
}

namespace Defaults {
    constexpr const char* DiscordBotApiToken = "Enter Discord bot API token here";
    constexpr bool YoutubeAuthEnabled = false;

    namespace Proxy {
        constexpr bool Enabled = false;
        constexpr const char* Url = "Enter proxy URL here";
    }
}

void Config::GenerateSampleFile() {
    json proxyObject;
    proxyObject[Objects::Proxy::Enabled] = Defaults::Proxy::Enabled;
    proxyObject[Objects::Proxy::Url] = Defaults::Proxy::Url;

    json configJson;
    configJson[Objects::DiscordBotApiToken] = Defaults::DiscordBotApiToken;
    configJson[Objects::Proxy::Object] = proxyObject;
    IO::WriteFile(Filename, configJson.dump(4) + '\n');
}

Config::Config() {
    std::string fileContents;
    try {
        fileContents = IO::ReadFile(Filename);
    }
    catch (...) {
        m_error = "Couldn't open config file";
    }

    try {
        const json configJson = json::parse(fileContents);
        m_discordBotApiToken = configJson.at(Objects::DiscordBotApiToken);
        m_youtubeAuthEnabled = configJson.at(Objects::YoutubeAuthEnabled);

        const json& proxyObject = configJson.at(Objects::Proxy::Object);
        m_proxyEnabled = proxyObject.at(Objects::Proxy::Enabled);
        m_proxyUrl = proxyObject.at(Objects::Proxy::Url);
    }
    catch (const json::exception&) {
        m_error = "Couldn't parse config file JSON";
        return;
    }
}

} // namespace kb
