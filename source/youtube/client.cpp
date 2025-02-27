#include "youtube/client.hpp"
using namespace kb::Youtube::ClientConst;

// STL modules
#include <algorithm>
#include <stdexcept>

// Boost libraries
#include <boost/regex.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "core/config.hpp"
#include "core/utility.hpp"

namespace kb {

/*
*   std::make_unique() needs public constructor, but the Youtube::Client class uses singleton pattern.
*   This is why operator new is used instead.
*/
const std::unique_ptr<Youtube::Client> Youtube::Client::Instance(new Youtube::Client);

/// @brief Generate UUID string
/// @param withoutDashes Whether to remove dashes from UUID or not
/// @return Generated UUID string
static std::string GenerateUuid(bool withoutDashes)
{
    std::string uuid = boost::uuids::to_string(boost::uuids::random_generator_mt19937()());
    if (withoutDashes)
    {
        uuid.erase(
            std::remove_if(
                uuid.begin(),
                uuid.end(),
                [](char character) { return character == '-'; }
            ), uuid.end()
        );
    }
    return uuid;
}

/// @brief Get current YouTube player ID
/// @throw std::runtime_error if internal error occurs
/// @return Current YouTube player ID
static std::string GetPlayerId()
{
    const Curl::Response iframeResponse = Curl::Get(Urls::IframeApi);
    if (iframeResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::GetPlayerId(): "
            "Couldn't get iframe API response [response code: {}]",
            iframeResponse.code
        ));
    }

    boost::smatch matches;
    if (!boost::regex_search(iframeResponse.data, matches, boost::regex(R"(https:\\\/\\\/www\.youtube\.com\\\/s\\\/player\\\/(.+?)\\\/)")))
        throw std::runtime_error("kb::Youtube::GetPlayerId(): Coudln't extract player ID from iframe API response");
    return matches.str(1);
}

/// @brief Convert client type to client name
/// @param clientType Client type
/// @throw std::invalid_argument if client type is unknown
/// @return Client name
static const char* TypeToName(Youtube::Client::Type clientType)
{
    switch (clientType)
    {
        case Youtube::Client::Type::Web:
            return "web";
        case Youtube::Client::Type::IOS:
            return "ios";
        case Youtube::Client::Type::Android:
            return "android";
        case Youtube::Client::Type::TvEmbedded:
            return "tv_embedded";
    }

    throw std::invalid_argument(fmt::format(
        "kb::Youtube::TypeToName(): "
        "Unknown client type [client type: {}]",
        static_cast<int>(clientType)
    ));
}

Youtube::Client::Client()
    : m_logger(Utility::CreateLogger("client"))
{
    try
    {
        m_clients = json::parse(ClientsData);
    }
    catch (const json::exception&)
    {
        m_error = "Couldn't parse clients JSON";
        return;
    }

    try
    {
        m_interpreter = std::make_unique<Interpreter>();
    }
    catch (const std::runtime_error& error)
    {
        m_error = fmt::format("Couldn't initialize JavaScript interpreter: \"{}\"", error.what());
        return;
    }
}

void Youtube::Client::updatePlayer()
{
    std::lock_guard lock(m_mutex);
    std::string currentPlayerId = GetPlayerId();
    if (m_playerId == currentPlayerId)
        return;
    m_playerId = currentPlayerId;

    const Curl::Response playerCodeResponse = Curl::Get(fmt::format(Urls::PlayerCode, m_playerId));
    if (playerCodeResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Client::update(): "
            "Couldn't get player code [player: \"{}\", response code: {}]",
            m_playerId, playerCodeResponse.code
        ));
    }

    boost::smatch matches;
    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(R"(signatureTimestamp:(\d+))")))
        throw std::runtime_error("kb::Youtube::Client::update(): Couldn't extract signature timestamp from player code");
    m_clients["tv_embedded"]["data"]["playbackContext"]["contentPlaybackContext"]["signatureTimestamp"] = std::stoi(matches.str(1));

    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(R"(=function\(.\)\{.=.\.split\(""\);([\w\$]+)\.[\s\S]*?return .\.join\(""\)\};)")))
        throw std::runtime_error("kb::Youtube::Client::update(): Couldn't extract signature decrypt function from player code");
    m_interpreter->reset();
    m_interpreter->execute(SignatureDecrypt + matches.str(0));

    std::string encapsulatedObjectName = boost::regex_replace(matches.str(1), boost::regex(R"(\$)"), R"(\\$)");
    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(fmt::format(R"(var {}=\{{[\s\S]*?\}};)", encapsulatedObjectName))))
        throw std::runtime_error("kb::Youtube::Client::update(): Couldn't extract singature decrypt object from player code");
    m_interpreter->execute(matches.str(0));
    m_logger.info("Updated to player \"{}\"", m_playerId);
}

Curl::Response Youtube::Client::requestApi(Type clientType, const std::string& requestMethod, json additionalData, bool updateNeedless)
{
    // TV embedded client needs current "signatureTimestamp" field to be passed in request body
    if (clientType == Type::TvEmbedded && !updateNeedless)
        updatePlayer();

    const char* clientName = TypeToName(clientType);
    additionalData.update(m_clients[clientName]["data"]);
    return Curl::Post(
        fmt::format(Urls::ApiRequest, requestMethod, m_clients[clientName]["api_key"].get<std::string>()),
        m_clients[clientName]["headers"], additionalData.dump()
    );
}

std::string Youtube::Client::decryptSignatureCipher(std::string signatureCipher)
{
    signatureCipher = m_interpreter->execute(fmt::format(R"(console.log(decodeURIComponent("{}")))", signatureCipher));
    boost::smatch matches;
    if (!boost::regex_match(signatureCipher, matches, boost::regex(R"(s=([\s\S]+)&sp=sig&url=([\s\S]+))")))
        throw std::runtime_error("kb::Youtube::Client::decryptSignatureCipher(): Couldn't extract signature and URL from signature cipher");

    updatePlayer();
    std::string decodedSignature = m_interpreter->execute(fmt::format(
        R"(console.log(encodeURIComponent({}(decodeURIComponent("{}")))))",
        SignatureDecrypt, matches.str(1)
    ));

    m_logger.info("Signature decrypted by player \"{}\"", m_playerId);
    return fmt::format("{}&sig={}", matches.str(2), decodedSignature);
}

} // namespace kb
