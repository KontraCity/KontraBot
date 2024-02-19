#include "bot/config.hpp"

namespace kc {

void Bot::Config::GenerateSampleFile()
{
    std::ofstream configFile(ConfigConst::ConfigFile);
    if (!configFile)
        throw std::runtime_error("kc::Bot::Config::GenerateSampleFile(): Couldn't create sample configuration file");

    json configJson;
    configJson[ConfigConst::DiscordBotApiTokenObject] = "Enter Discord bot API token here";
    configFile << configJson.dump(4) << '\n';
}

Bot::Config::Config()
{
    std::ifstream configFile(ConfigConst::ConfigFile);
    if (!configFile)
        throw std::runtime_error("kc::Bot::Config::Config(): Couldn't open configuration file");

    try
    {
        json configJson = json::parse(configFile);
        m_discordBotApiToken = configJson[ConfigConst::DiscordBotApiTokenObject];
    }
    catch (const json::exception&)
    {
        throw std::runtime_error("kc::Bot::Config::Config(): Couldn't parse configuration file JSON");
    }

    if (!boost::regex_match(m_discordBotApiToken, boost::regex(R"([\w]+\.[\w-]{6}\.[\w-]{38})")))
        throw std::runtime_error("kc::Bot::Config::Config(): Configuration file Discord bot API token is invalid");
}

} // namespace kc
