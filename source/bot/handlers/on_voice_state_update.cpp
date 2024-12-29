#include "bot/bot.hpp"

namespace kb {

void Bot::Bot::onVoiceStateUpdate(const dpp::voice_state_update_t& event)
{
    dpp::guild* guild = dpp::find_guild(event.state.guild_id);
    dpp::voiceconn* botVoice = event.from->get_voice(event.state.guild_id);

    std::lock_guard lock(m_mutex);
    Info info = updateInfoProcessedInteractions(guild->id);

    if (event.state.user_id != me.id)
    {
        if (botVoice && CountVoiceMembers(*guild, botVoice->channel_id) == 1)
            leaveVoice(event.from, *guild, info, Locale::EndReason::EverybodyLeft);
        return;
    }

    if (botVoice && botVoice->channel_id != event.state.channel_id)
    {
        info.stats().timesMoved += 1;
        leaveVoice(event.from, *guild, info, Locale::EndReason::Moved);
        return;
    }

    if (!event.state.channel_id.empty())
        return;

    /*
    *   Player instance is deleted when bot gracefully leaves voice channel.
    *   Bot was kicked if it wasn't deleted.
    */
    PlayerEntry playerEntry = m_players.find(event.state.guild_id);
    if (playerEntry != m_players.end())
    {
        info.stats().timesKicked += 1;
        playerEntry->second.endSession(info, Locale::EndReason::Kicked);
        m_players.erase(event.state.guild_id);
    }
}

} // namespace kb
