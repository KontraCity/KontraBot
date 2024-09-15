#include "youtube/client.hpp"
using namespace kc::Youtube::ClientConst;

namespace kc {

/*
*   std::make_unique() needs public constructor, but the Youtube::Client class uses singleton pattern.
*   This is why operator new is used instead.
*/
const std::unique_ptr<Youtube::Client> Youtube::Client::Instance(new Youtube::Client);

std::string Youtube::Client::GenerateUuid(bool withoutDashes)
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

std::string Youtube::Client::GetPlayerId()
{
    const Curl::Response iframeResponse = Curl::Get(Urls::IframeApi);
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
        case Type::IOS:
            return "ios";
        case Type::Android:
            return "android";
        case Type::TvEmbedded:
            return "tv_embedded";
    }

    throw std::invalid_argument(fmt::format(
        "kc::Youtube::Client::TypeToName(): "
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
            "kc::Youtube::Client::update(): "
            "Couldn't get player code [player: \"{}\", response code: {}]",
            m_playerId, playerCodeResponse.code
        ));
    }

    boost::smatch matches;
    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(R"(signatureTimestamp:(\d+))")))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract signature timestamp from player code");
    m_clients["tv_embedded"]["data"]["playbackContext"]["contentPlaybackContext"]["signatureTimestamp"] = std::stoi(matches.str(1));

    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(R"(=function\(a\)\{a=a\.split\(""\);([\w\$]+)\.[\s\S]*?return a\.join\(""\)\};)")))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract signature decrypt function from player code");
    m_interpreter->reset();
    m_interpreter->execute(SignatureDecrypt + matches.str(0));

    std::string encapsulatedObjectName = boost::regex_replace(matches.str(1), boost::regex(R"(\$)"), R"(\\$)");
    if (!boost::regex_search(playerCodeResponse.data, matches, boost::regex(fmt::format(R"(var {}=\{{[\s\S]*?\}};)", encapsulatedObjectName))))
        throw std::runtime_error("kc::Youtube::Client::update(): Couldn't extract singature decrypt object from player code");
    m_interpreter->execute(matches.str(0));
    m_logger.info("Updated to player \"{}\"", m_playerId);
}

void Youtube::Client::updateToken()
{
    const Curl::Response authTokenResponse = Curl::Post(
        Urls::AuthToken, { "Content-Type: application/json", "__Youtube_Oauth__: True" },
        json{
            {"client_id", Auth::ClientId},
            {"client_secret", Auth::ClientSecret},
            {"refresh_token", Cache::Instance->getYoutubeAuth().refreshToken},
            {"grant_type", "refresh_token"}
        }.dump()
    );
    if (authTokenResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Client::updateToken(): "
            "Couldn't get authorization token response [response code: {}]",
            authTokenResponse.code
        ));
    }

    try
    {
        const json authTokenJson = json::parse(authTokenResponse.data);
        if (authTokenJson.contains("error"))
        {
            std::string error = authTokenJson.at("error");
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Client::updateToken(): "
                "Unknown authorization token response error [error: \"{}\", response code: {}]",
                error, authTokenResponse.code
            ));
        }

        Cache::YoutubeAuth youtubeAuth = Cache::Instance->getYoutubeAuth();
        youtubeAuth.accessToken = authTokenJson.at("access_token");
        youtubeAuth.accessTokenType = authTokenJson.at("token_type");
        youtubeAuth.expiresAt = Utility::GetUnixTimestamp() + authTokenJson.at("expires_in").get<int>();
        Cache::Instance->setYoutubeAuth(youtubeAuth);
        m_logger.info("Access token refreshed successfully");
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Client::updateToken(): "
            "Couldn't parse authorization token response [id: {}]",
            error.id
        ));
    }
}

void Youtube::Client::checkAuthorization()
{
    if (!Config::Instance->youtubeAuthEnabled())
        return;
    if (Cache::Instance->getYoutubeAuth().authorized)
        return;

    const Curl::Response authCodeResponse = Curl::Post(
        Urls::AuthCode, { "Content-Type: application/json", "__Youtube_Oauth__: True" },
        json{
            {"client_id", Auth::ClientId},
            {"scope", Auth::ClientScopes},
            {"device_id", GenerateUuid(true)},
            {"device_model", "ytlr::"}
        }.dump()
    );
    if (authCodeResponse.code != 200)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Client::checkAuthorization(): "
            "Couldn't get authorization code response [response code: {}]",
            authCodeResponse.code
        ));
    }

    std::string deviceCode, userCode, verificationUrl;
    int interval = 0, expiresIn = 0;
    try
    {
        const json authCodeJson = json::parse(authCodeResponse.data);
        deviceCode = authCodeJson.at("device_code");
        userCode = authCodeJson.at("user_code");
        verificationUrl = authCodeJson.at("verification_url");
        interval = authCodeJson.at("interval");
        expiresIn = authCodeJson.at("expires_in");
    }
    catch (const json::exception& error)
    {
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Client::checkAuthorization(): "
            "Couldn't parse authorization code response JSON [id: {}]",
            error.id
        ));
    }

    m_logger.info("Authorization required");
    m_logger.info("Go to [{}] and enter the code \"{}\"", verificationUrl, userCode);
    for (int second = 0; second < expiresIn; second += interval)
    {
        Utility::Sleep(interval);
        const Curl::Response authTokenResponse = Curl::Post(
            Urls::AuthToken, { "Content-Type: application/json", "__Youtube_Oauth__: True" },
            json{
                {"client_id", Auth::ClientId},
                {"client_secret", Auth::ClientSecret},
                {"code", deviceCode},
                {"grant_type", "http://oauth.net/grant_type/device/1.0"}
            }.dump()
        );
        if (authTokenResponse.code != 200)
        {
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Client::checkAuthorization(): "
                "Couldn't get authorization token response [response code: {}]",
                authCodeResponse.code
            ));
        }

        try
        {
            const json authTokenJson = json::parse(authTokenResponse.data);
            if (authTokenJson.contains("error"))
            {
                std::string error = authTokenJson.at("error");
                if (error == "authorization_pending")
                    continue;
                throw std::runtime_error(fmt::format(
                    "kc::Youtube::Client::checkAuthorization(): "
                    "Unknown authorization token response error [error: \"{}\", response code: {}]",
                    error, authTokenResponse.code
                ));
            }

            Cache::YoutubeAuth youtubeAuth;
            youtubeAuth.authorized = true;
            youtubeAuth.accessToken = authTokenJson.at("access_token");
            youtubeAuth.accessTokenType = authTokenJson.at("token_type");
            youtubeAuth.expiresAt = Utility::GetUnixTimestamp() + authTokenJson.at("expires_in").get<int>();
            youtubeAuth.refreshToken = authTokenJson.at("refresh_token");
            Cache::Instance->setYoutubeAuth(youtubeAuth);

            m_logger.info("Authorized successfully");
            return;
        }
        catch (const json::exception& error)
        {
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Client::checkAuthorization(): "
                "Couldn't parse authorization token response [id: {}]",
                error.id
            ));
        }
    }

    throw std::runtime_error(fmt::format(
        "kc::Youtube::Client::checkAuthorization(): "
        "Couldn't authorize in {} seconds",
        expiresIn
    ));
}

Curl::Response Youtube::Client::requestApi(Type clientType, const std::string& requestMethod, json additionalData, bool updateNeedless)
{
    // TV embedded client needs current "signatureTimestamp" field to be passed in request body
    if (clientType == Type::TvEmbedded && !updateNeedless)
        updatePlayer();

    const char* clientName = TypeToName(clientType);
    additionalData.update(m_clients[clientName]["data"]);

    std::vector<std::string> headers = m_clients[clientName]["headers"];
    if (Config::Instance->youtubeAuthEnabled())
    {
        Cache::YoutubeAuth youtubeAuth = Cache::Instance->getYoutubeAuth();
        if (youtubeAuth.authorized)
        {
            // 10 second time reserve
            if (Utility::GetUnixTimestamp() + 10 >= youtubeAuth.expiresAt)
                updateToken();

            headers.push_back(fmt::format(
                "Authorization: {} {}",
                youtubeAuth.accessTokenType,
                youtubeAuth.accessToken
            ));
        }
    }

    return Curl::Post(
        fmt::format(Urls::ApiRequest, requestMethod, m_clients[clientName]["api_key"].get<std::string>()),
        headers, additionalData.dump()
    );
}

std::string Youtube::Client::decryptSignatureCipher(std::string signatureCipher)
{
    signatureCipher = m_interpreter->execute(fmt::format(R"(console.log(decodeURIComponent("{}")))", signatureCipher));
    boost::smatch matches;
    if (!boost::regex_match(signatureCipher, matches, boost::regex(R"(s=([\s\S]+)&sp=sig&url=([\s\S]+))")))
        throw std::runtime_error("kc::Youtube::Client::decryptSignatureCipher(): Couldn't extract signature and URL from signature cipher");

    updatePlayer();
    std::string decodedSignature = m_interpreter->execute(fmt::format(
        R"(console.log(encodeURIComponent({}(decodeURIComponent("{}")))))",
        SignatureDecrypt, matches.str(1)
    ));

    m_logger.info("Signature decrypted by player \"{}\"", m_playerId);
    return fmt::format("{}&sig={}", matches.str(2), decodedSignature);
}

} // namespace kc
