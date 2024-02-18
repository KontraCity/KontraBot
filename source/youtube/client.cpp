#include "youtube/client.hpp"

namespace kc {

/*
*   std::make_unique needs public constructor but Youtube::Client class uses singleton pattern.
*   This is why operator new is used instead.
*/
const std::unique_ptr<Youtube::Client> Youtube::Client::Instance(new Youtube::Client);

std::string Youtube::Client::GetPlayerId()
{
    const Curl::Response iframeResponse = Curl::Get(YoutubeIframeApiUrl);
    if (iframeResponse.code != 200)
        throw std::runtime_error("kc::Youtube::Client::GetPlayerId(): Couldn't get YouTube API response");

    boost::smatch matches;
    if (!boost::regex_search(iframeResponse.data, matches, boost::regex(R"(https:\\\/\\\/www\.youtube\.com\\\/s\\\/player\\\/(.+?)\\\/)")))
        throw std::runtime_error("kc::Youtube::Client::GetPlayerId(): Coudln't extract player ID");
    return matches.str(1);
}

const char* Youtube::Client::TypeToName(Type clientType)
{
    switch (clientType)
    {
        case Type::Web:
            return "web";
        case Type::Android:
            return "android";
        case Type::TvEmbedded:
            return "tv_embedded";
        default:
            throw std::runtime_error("kc::Youtube::Client::TypeToName(): Unknown client type");
    }
}

Youtube::Client::Client()
    : m_logger("client", std::make_shared<spdlog::sinks::stdout_color_sink_mt>())
{
    try
    {
        m_clients = json::parse(ClientsData);
        m_initialized = true;
    }
    catch (const json::exception&)
    {
        m_initialized = false;
    }
}

void Youtube::Client::update()
{
    std::lock_guard lock(m_mutex);
    if (!m_initialized)
        throw std::runtime_error("kc::Youtube::Client::update(): Not initialized");

    std::string currentPlayerId = GetPlayerId();
    if (m_playerId == currentPlayerId)
        return;
    m_playerId = currentPlayerId;

    const Curl::Response playerJsResponse = Curl::Get(fmt::format(YoutubePlayerJsUrl, m_playerId));
    if (playerJsResponse.code != 200)
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't get YouTube API response");

    boost::smatch matches;
    if (!boost::regex_search(playerJsResponse.data, matches, boost::regex(R"(signatureTimestamp:(\d+))")))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract signature timestamp");
    m_clients["tv_embedded"]["data"]["playbackContext"]["contentPlaybackContext"]["signatureTimestamp"] = std::stoi(matches.str(1));

    if (!boost::regex_search(playerJsResponse.data, matches, boost::regex(R"(=function\(a\)\{a=a\.split\(""\);(\w+)\.[\s\S]*?return a\.join\(""\)\};)")))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract signature decrypt function");
    m_interpreter.reset();
    m_interpreter.execute(SignatureDecryptFunction + matches.str(0));

    if (!boost::regex_search(playerJsResponse.data, matches, boost::regex(fmt::format(R"(var {0}=\{{[\s\S]*?\}};)", matches.str(1)))))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract singature decrypt object");
    m_interpreter.execute(matches.str(0));
    m_logger.info("Client updated to player \"{}\"", m_playerId);
}

Curl::Response Youtube::Client::requestApi(Type clientType, const std::string& requestMethod, json additionalData)
{
    // "tv_embedded" client needs actual "signatureTimestamp" field to be passed in request body
    if (clientType == Type::TvEmbedded)
        update();

    const char* clientName = TypeToName(clientType);
    additionalData.update(m_clients[clientName]["data"]);
    return Curl::Post(
        fmt::format(YoutubeApiRequestUrl, requestMethod, m_clients[clientName]["api_key"].get<std::string>()),
        m_clients[clientName]["headers"],
        additionalData.dump()
    );
}

std::string Youtube::Client::decryptSignatureCipher(std::string signatureCipher)
{
    signatureCipher = m_interpreter.execute(fmt::format(
        R"(console.log(decodeURIComponent("{}")))",
        signatureCipher
    ));

    boost::smatch matches;
    if (!boost::regex_match(signatureCipher, matches, boost::regex(R"(s=([\s\S]+)&sp=sig&url=([\s\S]+))")))
        throw std::runtime_error("kc::Youtube::Client::decryptSignatureCipher(): Couldn't extract signature and URL");

    update();
    std::string decodedSignature = m_interpreter.execute(fmt::format(
        R"(console.log(encodeURIComponent({}(decodeURIComponent("{}")))))",
        SignatureDecryptFunction, matches.str(1)
    ));

    m_logger.info("Signature decrypted by player \"{}\"", m_playerId);
    return fmt::format("{}&sig={}", matches.str(2), decodedSignature);
}

} // namespace kc
