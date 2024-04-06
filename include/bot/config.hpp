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
        // Bot configuration JSON file name
        constexpr const char* ConfigFile = "config.json";

        namespace Objects
        {
            constexpr const char* DiscordBotApiToken = "discord_bot_api_token";
        }

        namespace Defaults
        {
            constexpr const char* DiscordBotApiToken = "Enter Discord bot API token here";
        }
    }

    class Config
    {
    public:
        // Shared config instance pointer
        using Pointer = std::shared_ptr<Config>;

        // Configuration file read/parse error
        class Error : public std::logic_error
        {
        public:
            using logic_error::logic_error;
        };

    public:
        /// @brief Generate sample configuration file for user to fill out
        /// @throw std::runtime_error if file couldn't be created
        static void GenerateSampleFile();

    private:
        std::string m_discordBotApiToken;

    public:
        /// @brief Read and parse configuration file
        /// @throw kc::Bot::Config::Error if reading/parsing error occurs
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
