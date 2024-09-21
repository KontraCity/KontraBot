#include "bot/bot.hpp"

namespace kc {

void Bot::Bot::onVoiceReady(const dpp::voice_ready_t& event)
{
    std::lock_guard lock(m_mutex);
    Info info = updateInfoProcessedInteractions(event.voice_client->server_id);
    m_players.find(event.voice_client->server_id)->second.signalReady(info);
    m_logger.info("\"{}\": Voice client is ready", dpp::find_guild(event.voice_client->server_id)->name);
}

} // namespace kc
