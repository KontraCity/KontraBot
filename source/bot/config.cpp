#include "bot/config.hpp"

namespace kc {

void Bot::Config::GenerateSampleFile()
{
    std::ofstream configFile(ConfigConst::ConfigFile);
    if (!configFile)
    {
        throw std::runtime_error(fmt::format(
            "kc::Bot::Config::GenerateSampleFile(): "
            "Couldn't create sample configuration file \"{}\"",
            ConfigConst::ConfigFile
        ));
    }

    json configJson;
    configJson[ConfigConst::Objects::DiscordBotApiToken] = "Enter Discord bot API token here";
    configFile << configJson.dump(4) << '\n';
}

Bot::Config::Config()
{
    std::ifstream configFile(ConfigConst::ConfigFile);
    if (!configFile)
        throw Error(fmt::format("Couldn't open configuration file \"{}\"", ConfigConst::ConfigFile));

    try
    {
        json configJson = json::parse(configFile);
        m_discordBotApiToken = configJson[ConfigConst::Objects::DiscordBotApiToken];
    }
    catch (const json::exception& error)
    {
        throw Error(fmt::format("Couldn't parse configuration file JSON [id: {}]", error.id));
    }

    if (!boost::regex_match(m_discordBotApiToken, boost::regex(R"([\w]+\.[\w-]{6}\.[\w-]{38})")))
        throw Error(fmt::format("Discord bot API token \"{}\" is invalid", m_discordBotApiToken));
}

} // namespace kc
