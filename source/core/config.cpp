#include "core/config.hpp"
using namespace kb::ConfigConst;

// STL modules
#include <fstream>

// Library nlohmann::json
#include <nlohmann/json.hpp>

// Library Boost.Regex
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

/*
*   std::make_unique() needs public constructor, but the Config class uses singleton pattern.
*   This is why operator new is used instead.
*/
const std::unique_ptr<Config> Config::Instance(new Config);

void Config::GenerateSampleFile()
{
    std::ofstream configFile(ConfigFile);
    if (!configFile)
        throw std::runtime_error("kb::Bot::Config::GenerateSampleFile(): Couldn't create sample configuration file");

    json proxyObject;
    proxyObject[Objects::Proxy::Enabled] = Defaults::Proxy::Enabled;
    proxyObject[Objects::Proxy::Url] = Defaults::Proxy::Url;

    json configJson;
    configJson[Objects::DiscordBotApiToken] = Defaults::DiscordBotApiToken;
    configJson[Objects::YoutubeAuthEnabled] = Defaults::YoutubeAuthEnabled;
    configJson[Objects::Proxy::Object] = proxyObject;
    configFile << configJson.dump(4) << '\n';
}

Config::Config()
{
    std::ifstream configFile(ConfigFile);
    if (!configFile)
    {
        m_error = "Couldn't open configuration file";
        return;
    }

    try
    {
        json configJson = json::parse(configFile);
        m_discordBotApiToken = configJson.at(Objects::DiscordBotApiToken);
        m_youtubeAuthEnabled = configJson.at(Objects::YoutubeAuthEnabled);

        json proxyObject = configJson.at(Objects::Proxy::Object);
        m_proxyEnabled = proxyObject.at(Objects::Proxy::Enabled);
        m_proxyUrl = proxyObject.at(Objects::Proxy::Url);
    }
    catch (const json::exception&)
    {
        m_error = "Couldn't parse configuration file JSON";
        return;
    }

    if (!boost::regex_match(m_discordBotApiToken, boost::regex(R"([\w]+\.[\w-]{6}\.[\w-]{38})")))
    {
        m_error = fmt::format("Discord bot API token \"{}\" is invalid", m_discordBotApiToken);
        return;
    }
}

} // namespace kb
