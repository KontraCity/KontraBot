#include "youtube/client.hpp"

namespace kc {

/*
*   std::make_unique needs public constructor but Youtube::Client class uses singleton pattern.
*   This is why operator new is used instead.
*/
const std::unique_ptr<Youtube::Client> Youtube::Client::Instance(new Youtube::Client);

std::string Youtube::Client::GetPlayerId()
{
    const Curl::Response iframeResponse = Curl::Get(ClientConst::Urls::IframeApi);
    if (iframeResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Client::GetPlayerId(): "
            "Couldn't get iframe API response [response code: {}]",
            iframeResponse.code
        ));
    }

    boost::smatch matches;
    if (!boost::regex_search(iframeResponse.data, matches, boost::regex(R"(https:\\\/\\\/www\.youtube\.com\\\/s\\\/player\\\/(.+?)\\\/)")))
        throw std::runtime_error("kc::Youtube::Client::GetPlayerId(): Coudln't extract player ID from iframe API response");
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
        {
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Client::TypeToName(): "
                "Unknown client type [client type: {}]",
                static_cast<int>(clientType)
            ));
        }
    }
}

Youtube::Client::Client()
    : m_logger("client", std::make_shared<spdlog::sinks::stdout_color_sink_mt>())
    , m_initialized(true)
{
    try
    {
        m_clients = json::parse(ClientConst::ClientsData);
        m_interpreter = std::make_unique<JsInterpreter>();
        m_logger.info("Client initialized");
    }
    catch (...)
    {
        m_initialized = false;
        m_logger.error("Couldn't initialize client");
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

    const Curl::Response playerCodeResponse = Curl::Get(fmt::format(ClientConst::Urls::PlayerCode, m_playerId));
    if (playerCodeResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Client::update(): "
            "Couldn't get player code [player: \"{}\", response code: {}]",
            m_playerId, playerCodeResponse.code
        ));
    }
    
    boost::smatch matches;
    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(R"(signatureTimestamp:(\d+))")))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract signature timestamp from player code");
    m_clients["tv_embedded"]["data"]["playbackContext"]["contentPlaybackContext"]["signatureTimestamp"] = std::stoi(matches.str(1));

    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(R"(=function\(a\)\{a=a\.split\(""\);(\w+)\.[\s\S]*?return a\.join\(""\)\};)")))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract signature decrypt function from player code");
    m_interpreter->reset();
    m_interpreter->execute(ClientConst::SignatureDecryptFunction + matches.str(0));

    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(fmt::format(R"(var {}=\{{[\s\S]*?\}};)", matches.str(1)))))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract singature decrypt object from player code");
    m_interpreter->execute(matches.str(0));
    m_logger.info("Client updated to player \"{}\"", m_playerId);
}

Curl::Response Youtube::Client::requestApi(Type clientType, const std::string& requestMethod, json additionalData)
{
    // "tv_embedded" client needs current "signatureTimestamp" field to be passed in request body
    if (clientType == Type::TvEmbedded)
        update();

    const char* clientName = TypeToName(clientType);
    additionalData.update(m_clients[clientName]["data"]);
    return Curl::Post(
        fmt::format(ClientConst::Urls::ApiRequest, requestMethod, m_clients[clientName]["api_key"].get<std::string>()),
        m_clients[clientName]["headers"],
        additionalData.dump()
    );
}

std::string Youtube::Client::decryptSignatureCipher(std::string signatureCipher)
{
    signatureCipher = m_interpreter->execute(fmt::format(R"(console.log(decodeURIComponent("{}")))", signatureCipher));
    boost::smatch matches;
    if (!boost::regex_match(signatureCipher, matches, boost::regex(R"(s=([\s\S]+)&sp=sig&url=([\s\S]+))")))
        throw std::runtime_error("kc::Youtube::Client::decryptSignatureCipher(): Couldn't extract signature and URL from signature cipher");

    update();
    std::string decodedSignature = m_interpreter->execute(fmt::format(
        R"(console.log(encodeURIComponent({}(decodeURIComponent("{}")))))",
        ClientConst::SignatureDecryptFunction, matches.str(1)
    ));

    m_logger.info("Signature decrypted by player \"{}\"", m_playerId);
    return fmt::format("{}&sig={}", matches.str(2), decodedSignature);
}

} // namespace kc
