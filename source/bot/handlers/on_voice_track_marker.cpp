#include "bot/bot.hpp"

namespace kc {

void Bot::Bot::onVoiceTrackMarker(const dpp::voice_track_marker_t& event)
{
    const Signal signal(event.track_meta);
    switch (signal.type())
    {
        case Signal::Type::LivestreamSkipped:
        case Signal::Type::PremiereSkipped:
        case Signal::Type::PlayError:
        case Signal::Type::Played:
        case Signal::Type::ChapterReached:
        {
            std::lock_guard lock(m_mutex);
            Info info = updateInfoProcessedInteractions(event.voice_client->server_id);
            m_players.find(event.voice_client->server_id)->second.signalMarker(signal, info);
            return;
        }
        default:
        {
            m_logger.warn("Unknown voice track marker: \"{}\"", event.track_meta);
            return;
        }
    }
}

} // namespace kc
