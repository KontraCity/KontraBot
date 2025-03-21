#include "bot/bot.hpp"

namespace kb {

void Bot::Bot::onVoiceServerUpdate(const dpp::voice_server_update_t& event)
{
    const LogMessageFunction logMessage = [&event](const std::string& message)
    {
        return fmt::format(
            "\"{}\": {}",
            dpp::find_guild(event.guild_id)->name, message
        );
    };

    std::string playerEndpoint;
    {
        std::lock_guard lock(m_mutex);
        PlayerEntry playerEntry = m_players.find(event.guild_id);
        if (playerEntry == m_players.end())
        {
            m_logger.warn(logMessage("Voice server update event received from guild with no player"));
            return;
        }

        playerEndpoint = playerEntry->second.session().voiceServerEndpoint;
        if (playerEndpoint.empty())
        {
            playerEntry->second.updateVoiceServerEndpoint(event.endpoint);
            return;
        }

        if (playerEndpoint == event.endpoint)
            return;
        playerEntry->second.updateVoiceServerEndpoint(event.endpoint);
    }
    
    dpp::voiceconn* connection = event.from()->get_voice(event.guild_id);
    connection->disconnect();
    connection->websocket_hostname = event.endpoint;
    connection->connect(event.guild_id);
    m_logger.warn(logMessage(fmt::format("Voice server changed from \"{}\" to \"{}\", reconnecting", playerEndpoint, event.endpoint)));
}

} // namespace kb
