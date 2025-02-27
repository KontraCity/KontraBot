#include "core/config.hpp"
using namespace kb::ConfigConst;

#include <nlohmann/json.hpp>
using nlohmann::json;

#include "core/error.hpp"
#include "core/io.hpp"

namespace kb {

// std::make_unique won't work here: new is used instead
const std::unique_ptr<Config> Config::Instance(new Config);

void Config::GenerateSampleFile() {
    json proxyObject;
    proxyObject[Objects::Proxy::Enabled] = Defaults::Proxy::Enabled;
    proxyObject[Objects::Proxy::Url] = Defaults::Proxy::Url;

    json configJson;
    configJson[Objects::DiscordBotApiToken] = Defaults::DiscordBotApiToken;
    configJson[Objects::Proxy::Object] = proxyObject;
    IO::WriteFile(ConfigFile, configJson.dump(4) + '\n');
}

Config::Config() {
    std::string fileContents;
    try {
        fileContents = IO::ReadFile(ConfigFile);
    }
    catch (...) {
        m_error = "Couldn't open config file";
    }

    try {
        json configJson = json::parse(fileContents);
        m_discordBotApiToken = configJson.at(Objects::DiscordBotApiToken);

        json proxyObject = configJson.at(Objects::Proxy::Object);
        m_proxyEnabled = proxyObject.at(Objects::Proxy::Enabled);
        m_proxyUrl = proxyObject.at(Objects::Proxy::Url);
    }
    catch (const json::exception&) {
        m_error = "Couldn't parse config file JSON";
        return;
    }
}

} // namespace kb
