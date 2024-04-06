#include "bot/config.hpp"
using namespace kc::Bot::ConfigConst;

namespace kc {

void Bot::Config::GenerateSampleFile()
{
    std::ofstream configFile(ConfigFile);
    if (!configFile)
        throw std::runtime_error("kc::Bot::Config::GenerateSampleFile(): Couldn't create sample configuration file");

    json configJson;
    configJson[Objects::DiscordBotApiToken] = Defaults::DiscordBotApiToken;
    configFile << configJson.dump(4) << '\n';
}

Bot::Config::Config()
{
    std::ifstream configFile(ConfigFile);
    if (!configFile)
        throw Error(fmt::format("Couldn't open configuration file \"{}\"", ConfigFile));

    try
    {
        json configJson = json::parse(configFile);
        m_discordBotApiToken = configJson[Objects::DiscordBotApiToken];
    }
    catch (const json::exception&)
    {
        throw Error(fmt::format("Couldn't parse configuration JSON file \"{}\"", ConfigFile));
    }

    if (!boost::regex_match(m_discordBotApiToken, boost::regex(R"([\w]+\.[\w-]{6}\.[\w-]{38})")))
        throw Error(fmt::format("Discord bot API token \"{}\" is invalid", m_discordBotApiToken));
}

} // namespace kc
