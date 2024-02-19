#pragma once

// STL modules
#include <string>
#include <memory>
#include <fstream>

// Library nlohmann::json
#include <nlohmann/json.hpp>

// Library Boost.Regex
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

namespace kc {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Bot
{
    namespace ConfigConst
    {
        // Configuration JSON file
        constexpr const char* ConfigFile = "config.json";
        
        constexpr const char* DiscordBotApiTokenObject = "discord_bot_api_token";
    }

    class Config
    {
    public:
        // Config instance pointer
        using Pointer = std::shared_ptr<Config>;

    public:
        /// @brief Generate sample configuration file for user to fill in
        /// @throw std::runtime_error if internal error occurs
        static void GenerateSampleFile();

    private:
        std::string m_discordBotApiToken;

    public:
        /// @brief Read and parse configuration file
        /// @throw std::runtime_error if reading/parsing error occurs
        Config();

        /// @brief Get Discord bot API token
        /// @return Discord bot API token
        inline const std::string& discordBotApiToken() const
        {
            return m_discordBotApiToken;
        }
    };
}

} // namespace kc
