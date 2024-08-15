#include "common/config.hpp"
using namespace kc::ConfigConst;

namespace kc {

/*
*   std::make_unique() needs public constructor, but the Config class uses singleton pattern.
*   This is why operator new is used instead.
*/
const std::unique_ptr<Config> Config::Instance(new Config);

void Config::GenerateSampleFile()
{
    std::ofstream configFile(ConfigFile);
    if (!configFile)
        throw std::runtime_error("kc::Bot::Config::GenerateSampleFile(): Couldn't create sample configuration file");

    json proxyAuthObject;
    proxyAuthObject[Objects::Proxy::Auth::Required] = Defaults::Proxy::Auth::Required;
    proxyAuthObject[Objects::Proxy::Auth::User] = Defaults::Proxy::Auth::User;
    proxyAuthObject[Objects::Proxy::Auth::Password] = Defaults::Proxy::Auth::Password;

    json proxyObject;
    proxyObject[Objects::Proxy::Enabled] = Defaults::Proxy::Enabled;
    proxyObject[Objects::Proxy::Host] = Defaults::Proxy::Host;
    proxyObject[Objects::Proxy::Port] = Defaults::Proxy::Port;
    proxyObject[Objects::Proxy::Auth::Object] = proxyAuthObject;

    json configJson;
    configJson[Objects::DiscordBotApiToken] = Defaults::DiscordBotApiToken;
    configJson[Objects::Proxy::Object] = proxyObject;
    configFile << configJson.dump(4) << '\n';
}

Config::Config()
{
    std::ifstream configFile(ConfigFile);
    if (!configFile)
    {
        m_error = fmt::format("Couldn't open configuration file \"{}\"", ConfigFile);
        return;
    }

    try
    {
        json configJson = json::parse(configFile);
        m_discordBotApiToken = configJson[Objects::DiscordBotApiToken];

        json proxyObject = configJson[Objects::Proxy::Object];
        m_proxyEnabled = proxyObject[Objects::Proxy::Enabled];
        m_proxyHost = proxyObject[Objects::Proxy::Host];
        m_proxyPort = proxyObject[Objects::Proxy::Port];

        json proxyAuthObject = proxyObject[Objects::Proxy::Auth::Object];
        m_proxyAuthRequired = proxyAuthObject[Objects::Proxy::Auth::Required];
        m_proxyAuthUser = proxyAuthObject[Objects::Proxy::Auth::User];
        m_proxyAuthPassword = proxyAuthObject[Objects::Proxy::Auth::Password];
    }
    catch (const json::exception&)
    {
        m_error = fmt::format("Couldn't parse configuration JSON file \"{}\"", ConfigFile);
        return;
    }

    if (!boost::regex_match(m_discordBotApiToken, boost::regex(R"([\w]+\.[\w-]{6}\.[\w-]{38})")))
    {
        m_error = fmt::format("Discord bot API token \"{}\" is invalid", m_discordBotApiToken);
        return;
    }
}

} // namespace kc
