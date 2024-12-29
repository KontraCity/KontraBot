#pragma once

// STL modules
#include <string>
#include <memory>
#include <mutex>

// Library nlohmann/json
#include <nlohmann/json.hpp>

// Library spdlog
#include <spdlog/spdlog.h>

// Custom modules
#include "common/cache.hpp"
#include "common/curl.hpp"
#include "common/interpreter.hpp"

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Youtube
{
    namespace ClientConst
    {
        // JavaScript interpreter signature decryption function name
        constexpr const char* SignatureDecrypt = "_SingatureDecrypt";

        // Clients API keys, headers and POST data
        constexpr const char* ClientsData =
        {
            R"_delimeter(
{
    "web": {
        "api_key": "AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8",
        "headers": [
            "Origin: https://www.youtube.com",
            "Content-Type: application/json",
            "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.101 Safari/537.36,gzip(gfe)",
            "X-YouTube-Client-Name: 5",
            "X-YouTube-Client-Version: 17.33.2"
        ],
        "data": {
            "context": {
                "client": {
                    "hl": "en",
                    "gl": "RU",
                    "deviceMake": "",
                    "deviceModel": "",
                    "userAgent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36,gzip(gfe)",
                    "clientName": "WEB",
                    "clientVersion": "2.20230921.04.01",
                    "osName": "Windows",
                    "osVersion": "10.0",
                    "platform": "DESKTOP",
                    "clientFormFactor": "UNKNOWN_FORM_FACTOR",
                    "timeZone": "UTC",
                    "browserName": "Chrome",
                    "browserVersion": "95.0.4638.69",
                    "acceptHeader": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
                    "utcOffsetMinutes": 0
                },
                "user": {
                    "lockedSafetyMode": false
                },
                "request": {
                    "useSsl": true
                }
            }
        }
    },
    "ios": {
        "api_key": "AIzaSyB-63vPrdThhKuerbB2N_l7Kwwcxj6yUAc",
        "headers": [
            "Origin: https://www.youtube.com",
            "Content-Type: application/json",
            "User-Agent: com.google.ios.youtube/19.09.3 (iPhone14,3; U; CPU iOS 15_6 like Mac OS X)",
            "X-YouTube-Client-Name: 5",
            "X-YouTube-Client-Version: 19.09.3"
        ],
        "data": {
            "context": {
                "client": {
                    "clientName": "IOS",
                    "clientVersion": "19.09.3",
                    "deviceModel": "iPhone14,3",
                    "userAgent": "com.google.ios.youtube/19.09.3 (iPhone14,3; U; CPU iOS 15_6 like Mac OS X)",
                    "hl": "en",
                    "timeZone": "UTC",
                    "utcOffsetMinutes": 0
                }
            },
            "playbackContext": {
                "contentPlaybackContext": {
                    "html5Preference": "HTML5_PREF_WANTS"
                }
            },
            "contentCheckOk": true,
            "racyCheckOk": true
        }
    },
    "android": {
        "api_key": "AIzaSyA8eiZmM1FaDVjRy-df2KTyQ_vz_yYM39w",
        "headers": [
            "Origin: https://www.youtube.com",
            "Content-Type: application/json",
            "User-Agent: com.google.android.youtube/19.09.37 (Linux; U; Android 11) gzip",
            "X-YouTube-Client-Name: 3",
            "X-YouTube-Client-Version: 19.09.37"
        ],
        "data": {
            "context": {
                "client": {
                    "clientName": "ANDROID",
                    "clientVersion": "19.09.37",
                    "androidSdkVersion": 30,
                    "userAgent": "com.google.android.youtube/19.09.37 (Linux; U; Android 11) gzip",
                    "hl": "en",
                    "timeZone": "UTC",
                    "utcOffsetMinutes": 0
                }
            },
            "params": "CgIIAQ==",
            "playbackContext": {
                "contentPlaybackContext": {
                    "html5Preference": "HTML5_PREF_WANTS"
                }
            },
            "contentCheckOk": true,
            "racyCheckOk": true
        }
    },
    "tv_embedded": {
        "api_key": "AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8",
        "headers": [
            "Origin: https://www.youtube.com",
            "Content-Type: application/json",
            "X-YouTube-Client-Name: 85",
            "X-YouTube-Client-Version: 2.0"
        ],
        "data": {
            "context": {
                "client": {
                    "clientName": "TVHTML5_SIMPLY_EMBEDDED_PLAYER",
                    "clientVersion": "2.0",
                    "hl": "en",
                    "timeZone": "UTC",
                    "utcOffsetMinutes": 0
                },
                "thirdParty": {
                    "embedUrl": "https://www.youtube.com/"
                }
            },
            "playbackContext": {
                "contentPlaybackContext": {
                    "html5Preference": "HTML5_PREF_WANTS"
                }
            },
            "contentCheckOk": true,
            "racyCheckOk": true
        }
    }
}
            )_delimeter"
        };

        // Constants for YouTube OAuth2 authorization
        namespace Auth
        {
            constexpr const char* ClientId = "861556708454-d6dlm3lh05idd8npek18k6be8ba3oc68.apps.googleusercontent.com";
            constexpr const char* ClientSecret = "SboVhoG9s0rNafixCSGGKXAT";
            constexpr const char* ClientScopes = "http://gdata.youtube.com https://www.googleapis.com/auth/youtube";
        }

        namespace Urls
        {
            // YouTube OAuth2 authorization code URL
            constexpr const char* AuthCode = "https://www.youtube.com/o/oauth2/device/code";

            // YouTube OAuth2 authorization token URL
            constexpr const char* AuthToken = "https://www.youtube.com/o/oauth2/token";

            // YouTube iframe API URL
            constexpr const char* IframeApi = "https://www.youtube.com/iframe_api";

            // YouTube API reqeust URL
            // [0]: API request method
            // [1]: API key
            constexpr const char* ApiRequest = "https://www.youtube.com/youtubei/v1/{}?key={}&prettyPrint=false";

            // YouTube player code URL
            // [0]: Player ID
            constexpr const char* PlayerCode = "https://www.youtube.com/s/player/{}/player_ias.vflset/en_US/base.js";
        }
    }

    class Client
    {
    public:
        enum class Type
        {
            /*
            *   Simulates desktop browser access.
            *   Useful for getting verbose video/playlist information.
            */
            Web,

            /*
            *   Simulates Apple phone access.
            *   Useful for non age restricted videos audio extraction.
            */
            IOS,

            /*
            *   Simulates Android phone access.
            *   Useful for non age restricted videos audio extraction.
            */
            Android,

            /*
            *   Simulates smart TV access.
            *   Useful for age restricted videos audio extraction and searches.
            */
            TvEmbedded,
        };

    public:
        // Singleton instance
        static const std::unique_ptr<Client> Instance;

    private:
        spdlog::logger m_logger;
        std::mutex m_mutex;
        std::string m_error;
        json m_clients;
        std::string m_playerId;
        std::unique_ptr<Interpreter> m_interpreter;

    private:
        /// @brief Initialize client
        Client();

    private:
        /// @brief Update client interpreter and data
        /// @throw std::runtime_error if internal error occurs
        void updatePlayer();

        /// @brief Update YouTube authorization token
        /// @throw std::runtime_error if internal error occurs
        /// @return Updated YouTube authorization info
        Cache::YoutubeAuth updateToken();

    public:
        /// @brief Check YouTube user authorization and authorize if needed
        /// @throw std::runtime_error if internal error occurs
        void checkAuthorization();

        /// @brief Perform YouTube API request
        /// @param clientType Client type to use
        /// @param requestMethod Request method name
        /// @param additionalData Additional data to include in request body
        /// @param updateNeedless Whether or not client update is needless
        /// @return API response
        Curl::Response requestApi(Type clientType, const std::string& requestMethod, json additionalData = {}, bool updateNeedless = false);

        /// @brief Decrypt signature cipher string to URL
        /// @param signatureCipher Signature cipher string
        /// @throw std::runtime_error if internal error occurs
        /// @return Decrypted URL
        std::string decryptSignatureCipher(std::string signatureCipher);

    public:
        /// @brief Get YouTube client error message
        /// @return YouTube client error message (empty if no error)
        inline const std::string& error() const
        {
            return m_error;
        }
    };
}

} // namespace kb
