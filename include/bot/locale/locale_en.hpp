#pragma once

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale.hpp"
#include "common/utility.hpp"

namespace kc {

namespace Bot
{
    class LocaleEn : public Locale
    {
    public:
        /// @brief Get locale's count ending
        /// @param count Count to get ending for
        /// @return Locale's count ending
        static inline const char* GetEnding(unsigned int count)
        {
            if (count == 1)
                return "";
            return "s";
        }

    public:
        /// @brief Get locale type
        /// @return Locale type
        virtual inline Locale::Type LocaleType() final
        {
            return Locale::Type::English;
        }

        /// @brief Get locale name
        /// @return Locale name
        virtual inline const char* LocaleName() final
        {
            return "en";
        }

        /// @brief Generate "Joining voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string Joining(dpp::snowflake channelId) final
        {
            return fmt::format("Joining <#{}>", static_cast<uint64_t>(channelId));
        }

        /// @brief Generate "Already joined voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string AlreadyJoined(dpp::snowflake channelId) final
        {
            return fmt::format("Already joined <#{}>", static_cast<uint64_t>(channelId));
        }

        /// @brief Get "You are not in voice channel to join" message
        /// @return Message
        virtual inline const char* CantJoin() final
        {
            return "Join a voice channel first";
        }

        /// @brief Generate "Left voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string Left(dpp::snowflake channelId) final
        {
            return fmt::format("Left <#{}>", static_cast<uint64_t>(channelId));
        }

        /// @brief Get "Not sitting in voice channel to leave" message
        /// @return Message
        virtual inline const char* CantLeave() final
        {
            return "I'm not sitting anywhere";
        }

        /// @brief Get "Do you want to play whole playlist or just this video" message
        /// @return Message
        virtual inline const char* AmbigousPlay() final
        {
            return "Play whole playlist or just this video?";
        }

        /// @brief Get "Play video" message
        /// @return Message
        virtual inline const char* PlayVideo() final
        {
            return "Play video";
        }

        /// @brief Get "Play playlist" message
        /// @return Message
        virtual inline const char* PlayPlaylist() final
        {
            return "Play playlist";
        }

        /// @brief Get "Added to queue" message
        /// @return Message
        virtual inline const char* ItemAdded() final
        {
            return "Added to queue";
        }

        /// @brief Generate "Item not added to queue" message
        /// @param itemTitle Title of the item
        /// @return Generated message
        virtual inline std::string CouldntAdd(const std::string& itemTitle = "") final
        {
            if (itemTitle.empty())
                return "Couldn't add";
            return fmt::format("Couldn't add **{}**", itemTitle);
        }

        /// @brief Get "I can't play livestreams" message
        /// @return Message
        virtual inline const char* LivestreamsCantBePlayed() final
        {
            return "I can't play livestreams";
        }

        /// @brief Get "I can't play upcoming videos" message
        /// @return Message
        virtual inline const char* UpcomingVideosCantBePlayed() final
        {
            return "I can't play YouTube premieres";
        }

        /// @brief Get "It looks like this video is private" message
        /// @return Message
        virtual inline const char* YoutubePrivateVideo() final
        {
            return "It looks like this video is private";
        }

        /// @brief Get "It looks like this video is blocked" message
        /// @return Message
        virtual inline const char* YoutubeBlockedVideo() final
        {
            return "It looks like this video is blocked";
        }

        /// @brief Get "It looks like an internal YouTube error" message
        /// @return Message
        virtual inline const char* YoutubeInternalError() final
        {
            return "It looks like an internal YouTube error";
        }

        /// @brief Get "It looks like this playlist is private" message
        /// @return Message
        virtual inline const char* YoutubePrivatePlaylist() final
        {
            return "It looks like this playlist is private";
        }

        /// @brief Get "I don't know what happened" message
        /// @return Message
        virtual inline const char* YoutubeUnknownError() final
        {
            return "I don't know what happened";
        }

        /// @brief Get "This playlist contains only videos that I can't play" message
        /// @return Message
        virtual inline const char* PlaylistOfUnplayableVideos() final
        {
            return "Your playlist contains only videos that I can't play";
        }

        /// @brief Get "I can't play YouTube Shorts playlists" message
        /// @return Message
        virtual inline const char* ShortsPlaylistsCantBePlayed() final
        {
            return "I can't play YouTube Shorts playlists";
        }

        /// @brief Get "This playlist is empty" message
        /// @return Message
        virtual inline const char* EmptyPlaylist() final
        {
            return "This playlist is empty";
        }

        /// @brief Get "Something went wrong" message
        /// @return Message
        virtual inline const char* SomethingWentWrong() final
        {
            return "Something went wrong...";
        }

        /// @brief Get "What are we playing?" message
        /// @return Message
        virtual inline const char* WhatAreWePlaying() final
        {
            return "What are we playing?";
        }

        /// @brief Generate search result message
        /// @param count Count of items in search
        /// @return Generated message
        virtual inline std::string SearchResult(unsigned int count) final
        {
            if (count == 0)
                return "No results";
            return fmt::format("{} result{}", count, GetEnding(count));
        }

        /// @brief Generate search select menu option description
        /// @param item Item for option
        /// @return Generated description
        virtual inline std::string SearchOptionDescription(const Youtube::Item& item) final
        {
            switch (item.type())
            {
                case Youtube::Item::Type::Video:
                {
                    const Youtube::Video& video = std::get<Youtube::Video>(item);
                    return fmt::format("Video by {}, [{}]", video.author(), Utility::ToString(video.duration()));
                }
                case Youtube::Item::Type::Playlist:
                {
                    const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
                    return fmt::format(
                        "Playlist by {}, [{} video{}]",
                        playlist.author(),
                        Utility::ToString(playlist.videoCount()),
                        GetEnding(playlist.videoCount())
                    );
                }
            }

            return "";
        }

        /// @brief Generate "I don't understand this timestamp" message
        /// @param timestamp Timestamp in question
        /// @return Generated message
        virtual inline std::string IncorrectTimestamp(const std::string& timestamp) final
        {
            return fmt::format("\"{}\" is not a timestamp I can understand", timestamp);
        }

        /// @brief Get "I'm not playing to seek" message
        /// @return Message
        virtual inline const char* NotPlayingToSeek() final
        {
            return "I'm not playing to seek";
        }

        /// @brief Generate "Seeking to timestamp" message
        /// @param timestamp Timestamp in question
        /// @return Generated message
        virtual inline std::string SeekingTo(const std::string& timestamp) final
        {
            return fmt::format("Seeking to `{}`", timestamp);
        }

        /// @brief Get "I don't recognise this slashcommand" message
        /// @return Message
        virtual inline const char* UnknownSlashcommand() final
        {
            return "Sorry, I don't recognise this slashcommand";
        }

        /// @brief Get "I don't recognise this button" message
        /// @return Message
        virtual inline const char* UnknownButton() final
        {
            return "Sorry, I don't recognise this button";
        }

        /// @brief Get "I don't recognise this option" message
        /// @return Message
        virtual inline const char* UnknownOption() final
        {
            return "Sorry, I don't recognise this option";
        }

        /// @brief Generate "User used command" message
        /// @param username Name of user in question
        /// @param command The command used
        /// @return Generated message
        virtual inline std::string UsedCommand(const std::string& username, const std::string& command) final
        {
            return fmt::format("{} used /{}", username, command);
        }
    };
}

} // namespace kc
