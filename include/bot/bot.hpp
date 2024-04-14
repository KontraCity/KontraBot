#pragma once

// STL modules
#include <map>
#include <vector>
#include <functional>
#include <optional>

// Library DPP
#include <dpp/dpp.h>

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

/* Forward Bot class declaration for other modules */
namespace kc {
    namespace Bot {
        class Bot;
    }
}

// Custom kc::Bot modules
#include "bot/info/info.hpp"
#include "bot/locale/locale.hpp"
#include "bot/player/player.hpp"
#include "bot/player/session.hpp"
#include "bot/commands.hpp"
#include "bot/config.hpp"

// Other custom modules
#include "common/utility.hpp"

// Custom kc::Youtube modules
#include "youtube/item.hpp"
#include "youtube/search.hpp"

namespace kc {

namespace Bot
{
    class Bot : public dpp::cluster
    {
    private:
        // Function used to create handler's log message
        using LogMessageFunction = std::function<std::string(const std::string&)>;

        using PlayerEntry = std::map<dpp::snowflake, Player>::iterator;

        struct JoinStatus
        {
            enum class Result
            {
                Joined,                 // Successfully joined user's voice channel
                AlreadyJoined,          // Bot is already sitting in user's voice channel
                CantJoin,               // Bot is already sitting in another voice channel
                UserNotInVoiceChannel,  // User is not in sitting in voice channel
            };

            Result result;
            const dpp::channel* channel = nullptr;
        };

        struct LeaveStatus
        {
            enum class Result
            {
                Left,                   // Successfully left voice channel
                BotNotInVoiceChannel,   // Bot is not in voice channel to leave
            };

            Result result;
            const dpp::channel* channel = nullptr;
        };

        struct JoinJob
        {
            dpp::user requester;
            dpp::snowflake textChannelId;
            Youtube::Item item;
        };

    public:
        /// @brief Count voice members in voice channel
        /// @param guild Voice channel guild
        /// @param channelId ID of voice channel
        /// @return Count of voice members in voice channel
        static size_t CountVoiceMembers(const dpp::guild& guild, dpp::snowflake channelId);

    private:
        spdlog::logger m_logger;
        std::map<dpp::snowflake, Player> m_players;
        std::map<dpp::snowflake, std::string> m_ephemeralTokens;

    private:
        /// @brief Update player's text channel ID
        /// @param guildId ID of player's guild
        /// @param channelId ID of text channel to update
        /// @return Updated player entry
        PlayerEntry updatePlayerTextChannelId(dpp::snowflake guildId, dpp::snowflake channelId);

        /// @brief Check if bot's player controls are locked for user (only users sitting in the same voice channel with bot can control player)
        /// @param guild User's guild
        /// @param userId ID of user in question
        /// @return True if player controls are locked for user
        bool playerControlsLocked(const dpp::guild& guild, dpp::snowflake userId);

        /// @brief Join user's voice channel
        /// @param client Discord client handling event
        /// @param interaction The event to handle
        /// @param info Guild info
        /// @param item Start item requested by user
        /// @return Join status
        JoinStatus joinUserVoice(dpp::discord_client* client, const dpp::interaction& interaction, Info& info, const Youtube::Item& item = {});

        /// @brief Add item to queue
        /// @param client Discord client handling event
        /// @param interaction The event to handle
        /// @param itemId ID of item requested by user
        /// @param logMessage Handler log message function
        /// @param info Guild's info
        /// @param showRequester Whether or not to show requester in result message
        /// @return Result message
        dpp::message addItem(dpp::discord_client* client, const dpp::interaction& interaction, const std::string& itemId, const LogMessageFunction& logMessage, Info& info, bool showRequester = false);

    public:
        /// @brief Initialize bot
        /// @param config Initialized config
        /// @param registerCommands Wherther or not to register commands and exit
        Bot(Config::Pointer config, bool registerCommands = false);

        /// @brief Leave voice channel
        /// @param client Discord client serving guild
        /// @param guild Voice channel's guild
        /// @param info Guild's info
        /// @param reason Leave reason
        /// @return Leave status
        LeaveStatus leaveVoice(dpp::discord_client* client, const dpp::guild& guild, Info& info, Locale::EndReason reason = Locale::EndReason::UserRequested);
    };
}

} // namespace kc
