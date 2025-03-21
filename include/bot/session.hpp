#pragma once

// STL modules
#include <optional>

// Library DPP
#include <dpp/dpp.h>

// Boost libraries
#include <boost/date_time.hpp>

#include <ytcpp/item.hpp>

namespace kb {

namespace Bot
{
    struct Session
    {
        struct EnqueuedItem
        {
            ytcpp::Item item;
            dpp::user requester;
        };

        struct PlayingVideo
        {
            ytcpp::Video video;
            /* Temporarily unsupported!
            Youtube::Video::Chapter chapter;
            */
        };

        struct PlayingPlaylist
        {
            ytcpp::Playlist playlist;
            ytcpp::Playlist::Iterator iterator;
        };

        // Discord members
        const dpp::snowflake guildId;
        const dpp::snowflake voiceChannelId;
        dpp::snowflake textChannelId;

        // Common members
        const uint64_t number;
        const dpp::user starter;
        pt::ptime startTimestamp;
        uint64_t tracksPlayed = 0;
        std::string voiceChannelStatus;
        std::string voiceServerEndpoint;

        // Queueing members
        std::optional<PlayingVideo> playingVideo;
        std::optional<PlayingPlaylist> playingPlaylist;
        std::optional<dpp::user> playingRequester;
        std::deque<EnqueuedItem> queue;
        int64_t seekTimestamp = -1;
    };
}

} // namespace kb
