#pragma once

// STL modules
#include <optional>

// Library DPP
#include <dpp/dpp.h>

// Custom modules
#include "bot/signal.hpp"
#include "bot/locale/locale.hpp"
#include "common/utility.hpp"
#include "youtube/item.hpp"
#include "youtube/search.hpp"

namespace kc {

namespace Bot
{
    namespace Embeds
    {
        namespace Statuses
        {
            namespace Success
            {
                constexpr uint32_t Color = dpp::colors::dark_green;
                constexpr const char* Prefix = u8"✅ ";
            }

            namespace Problem
            {
                constexpr uint32_t Color = dpp::colors::red;
                constexpr const char* Prefix = u8"❌ ";
            }

            namespace Question
            {
                constexpr uint32_t Color = dpp::colors::yellow;
                constexpr const char* Prefix = u8"❔ ";
            }
        }

        /// @brief Create "Joined voice channel" embed
        /// @param locale Locale to create embed for
        /// @param channelId ID of voice channel
        /// @return Created embed in a message
        dpp::message Joining(const Locale::Pointer& locale, dpp::snowflake channelId);

        /// @brief Create "Already joined voice channel" embed
        /// @param locale Locale to create embed for
        /// @param channelId ID of voice channel
        /// @return Created embed in a message
        dpp::message AlreadyJoined(const Locale::Pointer& locale, dpp::snowflake channelId);

        /// @brief Create "You are not in voice channel to join" embed
        /// @param locale Locale to create embed for
        /// @return Created embed in an ephemeral message
        dpp::message CantJoin(const Locale::Pointer& locale);

        /// @brief Create "Left voice channel" embed
        /// @param locale Locale to create embed for
        /// @param channelId ID of voice channel
        /// @return Created embed in a message
        dpp::message Left(const Locale::Pointer& locale, dpp::snowflake channelId);

        /// @brief Create "Not sitting in voice channel to leave" embed
        /// @param locale Locale to create embed for
        /// @return Created embed in an ephemeral message
        dpp::message CantLeave(const Locale::Pointer& locale);

        /// @brief Create "Do you want to play whole playlist or just this video?" embed
        /// @param locale Locale to create embed for
        /// @param videoId Video ID
        /// @param playlistId Playlist ID
        /// @return Created embed in an ephemeral message with buttons
        dpp::message AmbigousPlay(const Locale::Pointer& locale, const std::string& videoId, const std::string& playlistId);

        /// @brief Create "Item added to queue" embed
        /// @param locale Locale to create embed for
        /// @param item Added item
        /// @return Created embed in a message
        dpp::message ItemAdded(const Locale::Pointer& locale, const Youtube::Item& item, const std::optional<dpp::user>& requester = {});

        /// @brief Create "I can't play livestreams" embed
        /// @param locale Locale to create embed for
        /// @param livestream Livestream in question
        /// @param requester Requester user info to be shown in embed if needed
        /// @return Created embed in an ephemeral message
        dpp::message LivestreamsCantBePlayed(const Locale::Pointer& locale, const Youtube::Video& livestream, const std::optional<dpp::user>& requester = {});

        /// @brief Create "I can't play upcoming videos" embed
        /// @param locale Locale to create embed for
        /// @param upcomingVideo Upcoming video in question
        /// /// @param requester Requester user info to be shown in embed if needed
        /// @return Created embed in an ephemeral message
        dpp::message UpcomingVideosCantBePlayed(const Locale::Pointer& locale, const Youtube::Video& upcomingVideo, const std::optional<dpp::user>& requester = {});

        /// @brief Create "Couldn't add item because of YouTube error" embed
        /// @param locale Locale to create embed for
        /// @param error Error in question
        /// @return Created embed in an ephemeral message
        dpp::message CouldntAddYoutubeError(const Locale::Pointer& locale, const Youtube::YoutubeError& error);

        /// @brief Create "Couldn't add item because of local error" embed
        /// @param locale Locale to create embed for
        /// @param error Error in question
        /// @return Created embed in an ephemeral message
        dpp::message CouldntAddLocalError(const Locale::Pointer& locale, const Youtube::LocalError& error);

        /// @brief Create "Couldn't add item because of unknown error" embed
        /// @param locale Locale to create embed for
        /// @return Created embed in an ephemeral message
        dpp::message CouldntAddUnknownError(const Locale::Pointer& locale);

        /// @brief Create search result embed
        /// @param locale Locale to create embed for
        /// @param searchResult Search result
        /// @return Created embed in an ephemeral message with select menu
        dpp::message Search(const Locale::Pointer& locale, const Youtube::SearchResult& searchResult);

        /// @brief Create bad seek embed
        /// @param locale Locale to create embed for
        /// @param timestampString Seek timestamp string
        /// @return Created embed in an ephemeral message
        dpp::message BadSeek(const Locale::Pointer& locale, const std::string& timestampString);

        /// @brief Create "Not playing to seek" embed
        /// @param locale Locale to create embed for
        /// @return Created embed in an ephemeral message
        dpp::message NotPlayingToSeek(const Locale::Pointer& locale);

        /// @brief Create "Seeking to timestamp" embed
        /// @param locale Locale to create embed for
        /// @param timestamp Timestamp seeking to
        /// @return Created embed in a message
        dpp::message SeekingTo(const Locale::Pointer& locale, const std::string& timestamp);

        /// @brief Create "I don't recognise this slashcommand" embed
        /// @param locale Locale to create embed for
        /// @return Created embed in an ephemeral message
        dpp::message UnknownSlashcommand(const Locale::Pointer& locale);

        /// @brief Create "I don't recognise this button" embed
        /// @param locale Locale to create embed for
        /// @return Created embed in an ephemeral message
        dpp::message UnknownButton(const Locale::Pointer& locale);

        /// @brief Create "I don't recognise this option" embed
        /// @param locale Locale to create embed for
        /// @return Created embed in an ephemeral message
        dpp::message UnknownOption(const Locale::Pointer& locale);
    }
}

} // namespace kc
