#pragma once

// STL modules
#include <map>
#include <vector>
#include <functional>
#include <optional>
#include <mutex>

// Library DPP
#include <dpp/dpp.h>

// Library {fmt}
#include <fmt/format.h>

/* Forward Bot class declaration for other modules */
namespace kc {
    namespace Bot {
        class Bot;
    }
}

// Custom kc::Bot modules
#include "bot/locale/locale.hpp"
#include "bot/commands.hpp"
#include "bot/info.hpp"
#include "bot/player.hpp"
#include "bot/session.hpp"

// Other custom modules
#include "common/config.hpp"
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

        // Bot player map entry
        using PlayerEntry = std::map<dpp::snowflake, Player>::iterator;

        struct JoinStatus
        {
            enum class Result
            {
                Joined,                 // Successfully joined user's voice channel
                AlreadyJoined,          // Bot is already sitting in user's voice channel
                CantJoin,               // Bot is already sitting in another voice channel
                UserNotInVoiceChannel,  // User is not in sitting in voice channel
                UserInAfkChannel,       // User is sitting in an AFK channel
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
        std::mutex m_mutex;
        std::thread m_presenceThread;
        std::map<dpp::snowflake, Player> m_players;
        std::map<dpp::snowflake, std::string> m_ephemeralTokens;

    private:
        /// @brief Presence thread implementation
        void presenceFunction();

        void updateEphemeralToken(const dpp::confirmation_callback_t& confirmationEvent, std::string token);

        /// @brief Update player's text channel ID
        /// @param guildId ID of player's guild
        /// @param channelId ID of text channel to update
        /// @return Updated player entry
        PlayerEntry updatePlayerTextChannelId(dpp::snowflake guildId, dpp::snowflake channelId);

        /// @brief Update info's processed interactions
        /// @param guildId ID of info's guild
        /// @return Updated info
        Info updateInfoProcessedInteractions(dpp::snowflake guildId);

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

    private:
        /// @brief Handle autocomplete event
        /// @param event The event to handle
        void onAutocomplete(const dpp::autocomplete_t& event);

        /// @brief Handle button click event
        /// @param event The event to handle
        void onButtonClick(const dpp::button_click_t& event);

        /// @brief Handle log event
        /// @param event The event to handle
        /// @param registerCommands Whether bot was launched in command registration mode or not
        void onLog(const dpp::log_t& event, bool registerCommands);

        /// @brief Handle message create event
        /// @param event The event to handle
        void onMessageCreate(const dpp::message_create_t& event);

        /// @brief Handle ready event
        /// @param event The event to handle
        void onReady(const dpp::ready_t& event);

        /// @brief Handle select click event
        /// @param event The event to handle
        void onSelectClick(const dpp::select_click_t& event);

        /// @brief Handle slashcommand event
        /// @param event The event to handle
        void onSlashcommand(const dpp::slashcommand_t& event);

        /// @brief Handle voice ready event
        /// @param event The event to handle
        void onVoiceReady(const dpp::voice_ready_t& event);

        /// @brief Handle voice server update event
        /// @param event The event to handle
        void onVoiceServerUpdate(const dpp::voice_server_update_t& event);

        /// @brief Handle voice state update event
        /// @param event The event to handle
        void onVoiceStateUpdate(const dpp::voice_state_update_t& event);

        /// @brief Handle voice track marker event
        /// @param event The event to handle
        void onVoiceTrackMarker(const dpp::voice_track_marker_t& event);

    public:
        /// @brief Initialize bot
        /// @param registerCommands Wherther or not to register commands and exit
        Bot(bool registerCommands = false);

        /// @brief Leave voice channel
        /// @param client Discord client serving guild
        /// @param guild Voice channel's guild
        /// @param info Guild's info
        /// @param reason Leave reason
        /// @return Leave status
        LeaveStatus leaveVoice(dpp::discord_client* client, const dpp::guild& guild, Info& info, Locale::EndReason reason);
    };
}

} // namespace kc
