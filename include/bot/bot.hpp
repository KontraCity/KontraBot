#pragma once

// STL modules
#include <map>
#include <vector>
#include <functional>

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
#include "bot/locale/locale_ru.hpp"
#include "bot/commands.hpp"
#include "bot/config.hpp"
#include "bot/info.hpp"
#include "bot/player.hpp"

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

        struct JoinStatus
        {
            enum class Result
            {
                Joined,
                Joining,
                AlreadyJoined,
                CantJoin,
            };

            Result result;
            const dpp::channel* channel = nullptr;
        };

        struct LeaveStatus
        {
            enum class Result
            {
                Left,
                CantLeave,
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

    private:
        /// @brief Count voice members in voice channel
        /// @param guild Voice channel guild
        /// @param channelId ID of voice channel
        /// @return Count of voice members in voice channel
        static size_t CountVoiceMembers(const dpp::guild& guild, dpp::snowflake channelId);

        /// @brief Check if user is in voice channel
        /// @param guild Voice channel guild
        /// @param channelId ID of voice channel
        /// @param userId ID of user
        /// @return True if user is in voice channel
        static bool UserInVoiceChannel(const dpp::guild& guild, dpp::snowflake channelId, dpp::snowflake userId);

    private:
        spdlog::logger m_logger;
        std::map<dpp::snowflake, Player> m_players;
        std::map<dpp::snowflake, JoinJob> m_joinJobs;
        std::map<dpp::snowflake, std::string> m_ephemeralTokens;

    private:
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
        /// @param showRequester Whether or not to show requester in result message
        /// @return Result message
        dpp::message addItem(dpp::discord_client* client, const dpp::interaction& interaction, const std::string& itemId, const LogMessageFunction& logMessage, bool showRequester = false);

    public:
        /// @brief Initialize bot
        /// @param config Initialized config
        /// @param registerCommands Wherther or not to register commands and exit
        Bot(Config::Pointer config, bool registerCommands = false);

        /// @brief Leave voice channel
        /// @param client Discord client serving guild
        /// @param guild Voice channel's guild
        /// @param reason Leave reason
        /// @return Leave status
        LeaveStatus leaveVoice(dpp::discord_client* client, const dpp::guild& guild, Locale::EndReason reason = Locale::EndReason::None);
    };
}

} // namespace kc
