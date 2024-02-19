#pragma once

// STL modules
#include <string>
#include <memory>

// Library DPP
#include <dpp/dpp.h>

// Custom modules
#include "youtube/item.hpp"

namespace kc {

namespace Bot
{
    class Locale
    {
    public:
        // Locale instance pointer
        using Pointer = std::unique_ptr<Locale>;

        enum class Type
        {
            English,
            Russian,
        };

    public:
        /// @brief Get locale type
        /// @return Locale type
        virtual inline Type LocaleType() = 0;

        /// @brief Get locale name
        /// @return Locale name
        virtual inline const char* LocaleName() = 0;

        /// @brief Generate "Joining voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string Joining(dpp::snowflake channelId) = 0;

        /// @brief Generate "Already joined voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string AlreadyJoined(dpp::snowflake channelId) = 0;

        /// @brief Get "You are not in voice channel to join" message
        /// @return Message
        virtual inline const char* CantJoin() = 0;

        /// @brief Generate "Left voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string Left(dpp::snowflake channelId) = 0;

        /// @brief Get "Not sitting in voice channel to leave" message
        /// @return Message
        virtual inline const char* CantLeave() = 0;

        /// @brief Get "Do you want to play whole playlist or just this video" message
        /// @return Message
        virtual inline const char* AmbigousPlay() = 0;

        /// @brief Get "Play video" message
        /// @return Message
        virtual inline const char* PlayVideo() = 0;

        /// @brief Get "Play playlist" message
        /// @return Message
        virtual inline const char* PlayPlaylist() = 0;

        /// @brief Get "Added to queue" message
        /// @return Message
        virtual inline const char* ItemAdded() = 0;

        /// @brief Generate "Item not added to queue" message
        /// @param itemTitle Title of the item
        /// @return Generated message
        virtual inline std::string CouldntAdd(const std::string& itemTitle = "") = 0;

        /// @brief Get "I can't play livestreams" message
        /// @return Message
        virtual inline const char* LivestreamsCantBePlayed() = 0;

        /// @brief Get "I can't play upcoming videos" message
        /// @return Message
        virtual inline const char* UpcomingVideosCantBePlayed() = 0;

        /// @brief Get "It looks like this video is private" message
        /// @return Message
        virtual inline const char* YoutubePrivateVideo() = 0;

        /// @brief Get "It looks like this video is blocked" message
        /// @return Message
        virtual inline const char* YoutubeBlockedVideo() = 0;

        /// @brief Get "It looks like an internal YouTube error" message
        /// @return Message
        virtual inline const char* YoutubeInternalError() = 0;

        /// @brief Get "It looks like this playlist is private" message
        /// @return Message
        virtual inline const char* YoutubePrivatePlaylist() = 0;

        /// @brief Get "I don't know what happened" message
        /// @return Message
        virtual inline const char* YoutubeUnknownError() = 0;

        /// @brief Get "This playlist contains only videos that I can't play" message
        /// @return Message
        virtual inline const char* PlaylistOfUnplayableVideos() = 0;

        /// @brief Get "I can't play YouTube Shorts playlists" message
        /// @return Message
        virtual inline const char* ShortsPlaylistsCantBePlayed() = 0;

        /// @brief Get "This playlist is empty" message
        /// @return Message
        virtual inline const char* EmptyPlaylist() = 0;

        /// @brief Get "Something went wrong" message
        /// @return Message
        virtual inline const char* SomethingWentWrong() = 0;

        /// @brief Get "What are we playing?" message
        /// @return Message
        virtual inline const char* WhatAreWePlaying() = 0;

        /// @brief Generate search result message
        /// @param count Count of items in search
        /// @return Generated message
        virtual inline std::string SearchResult(unsigned int count) = 0;

        /// @brief Generate search select menu option description
        /// @param item Item for option
        /// @return Generated description
        virtual inline std::string SearchOptionDescription(const Youtube::Item& item) = 0;

        /// @brief Generate "I don't understand this timestamp" message
        /// @param timestamp Timestamp in question
        /// @return Generated message
        virtual inline std::string IncorrectTimestamp(const std::string& timestamp) = 0;

        /// @brief Get "I'm not playing to seek" message
        /// @return Message
        virtual inline const char* NotPlayingToSeek() = 0;

        /// @brief Generate "Seeking to timestamp" message
        /// @param timestamp Timestamp in question
        /// @return Generated message
        virtual inline std::string SeekingTo(const std::string& timestamp) = 0;

        /// @brief Get "I don't recognise this slashcommand" message
        /// @return Message
        virtual inline const char* UnknownSlashcommand() = 0;

        /// @brief Get "I don't recognise this button" message
        /// @return Message
        virtual inline const char* UnknownButton() = 0;

        /// @brief Get "I don't recognise this option" message
        /// @return Message
        virtual inline const char* UnknownOption() = 0;

        /// @brief Generate "User used command" message
        /// @param username Name of user in question
        /// @param command The command used
        /// @return Generated message
        virtual inline std::string UsedCommand(const std::string& username, const std::string& command) = 0;
    };
}

} // namespace kc
