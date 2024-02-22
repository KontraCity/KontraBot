#pragma once

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale.hpp"
#include "common/utility.hpp"

namespace kc {

namespace Bot
{
    namespace LocaleRuConst
    {
        constexpr Locale::Type Type = Locale::Type::Russian;
        constexpr const char* Name = "ru";
    }

    class LocaleRu : public Locale
    {
    public:
        /// @brief Get locale's count ending
        /// @param count Count to get ending for
        /// @return Locale's count ending
        static inline const char* GetEnding(unsigned int count)
        {
            switch (count)
            {
                case 1:
                    return u8"";
                case 2:
                case 3:
                case 4:
                    return u8"а";
                default:
                    return u8"ов";
            }
        }

    public:
        /// @brief Get locale type
        /// @return Locale type
        virtual inline Locale::Type LocaleType() final
        {
            return LocaleRuConst::Type;
        }

        /// @brief Get locale name
        /// @return Locale name
        virtual inline const char* LocaleName() final
        {
            return LocaleRuConst::Name;
        }

        /// @brief Generate "Joining voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string Joining(dpp::snowflake channelId) final
        {
            return fmt::format(u8"Захожу в <#{}>", static_cast<uint64_t>(channelId));
        }

        /// @brief Generate "Already joined voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string AlreadyJoined(dpp::snowflake channelId) final
        {
            return fmt::format(u8"Уже сижу в <#{}>", static_cast<uint64_t>(channelId));
        }

        /// @brief Get "You are not in voice channel to join" message
        /// @return Message
        virtual inline const char* CantJoin() final
        {
            return u8"Сначала подключись к голосовому каналу";
        }

        /// @brief Generate "Left voice channel" message
        /// @param channelId ID of voice channel
        /// @return Generated message
        virtual inline std::string Left(dpp::snowflake channelId) final
        {
            return fmt::format(u8"Ушёл из <#{}>", static_cast<uint64_t>(channelId));
        }

        /// @brief Get "Not sitting in voice channel to leave" message
        /// @return Message
        virtual inline const char* CantLeave() final
        {
            return u8"И так нигде не сижу";
        }

        /// @brief Get "Do you want to play whole playlist or just this video" message
        /// @return Message
        virtual inline const char* AmbigousPlay() final
        {
            return u8"Играть весь плейлист или только это видео?";
        }

        /// @brief Get "Play video" message
        /// @return Message
        virtual inline const char* PlayVideo() final
        {
            return u8"Играть видео";
        }

        /// @brief Get "Play playlist" message
        /// @return Message
        virtual inline const char* PlayPlaylist() final
        {
            return u8"Играть плейлист";
        }

        /// @brief Get "Added to queue" message
        /// @return Message
        virtual inline const char* ItemAdded() final
        {
            return u8"Добавлено в очердь";
        }

        /// @brief Generate "Item not added to queue" message
        /// @param itemTitle Title of the item
        /// @return Generated message
        virtual inline std::string CouldntAdd(const std::string& itemTitle = "") final
        {
            if (itemTitle.empty())
                return u8"Не получилось добавить";
            return fmt::format(u8"Не получилось добавить **{}**", itemTitle);
        }

        /// @brief Get "I can't play livestreams" message
        /// @return Message
        virtual inline const char* LivestreamsCantBePlayed() final
        {
            return u8"Я не умею играть стримы";
        }

        /// @brief Get "I can't play upcoming videos" message
        /// @return Message
        virtual inline const char* UpcomingVideosCantBePlayed() final
        {
            return u8"Я не умею играть премьеры";
        }

        /// @brief Get "It looks like this video is private" message
        /// @return Message
        virtual inline const char* YoutubePrivateVideo() final
        {
            return u8"Кажется, у этог видео ограниченный доступ";
        }

        /// @brief Get "It looks like this video is blocked" message
        /// @return Message
        virtual inline const char* YoutubeBlockedVideo() final
        {
            return u8"Кажется, это видео заблокировано";
        }

        /// @brief Get "It looks like an internal YouTube error" message
        /// @return Message
        virtual inline const char* YoutubeInternalError() final
        {
            return u8"Кажется, это ошибка на стороне YouTube";
        }

        /// @brief Get "It looks like this playlist is private" message
        /// @return Message
        virtual inline const char* YoutubePrivatePlaylist() final
        {
            return u8"Кажется, у этого плейлиста ограниченный доступ";
        }

        /// @brief Get "I don't know what happened" message
        /// @return Message
        virtual inline const char* YoutubeUnknownError() final
        {
            return u8"Я не знаю, что произошло";
        }

        /// @brief Get "This playlist contains only videos that I can't play" message
        /// @return Message
        virtual inline const char* PlaylistOfUnplayableVideos() final
        {
            return u8"Твой плейлист содержит только те видео, которые я не умею играть";
        }

        /// @brief Get "I can't play YouTube shorts playlists" message
        /// @return Message
        virtual inline const char* ShortsPlaylistsCantBePlayed() final
        {
            return u8"Я не умею играть плейлисты YouTube Shorts";
        }

        /// @brief Get "This playlist is empty" message
        /// @return Message
        virtual inline const char* EmptyPlaylist() final
        {
            return u8"Этот плейлист пуст";
        }

        /// @brief Get "Something went wrong" message
        /// @return Message
        virtual inline const char* SomethingWentWrong() final
        {
            return u8"Что-то пошло не так...";
        }

        /// @brief Get "What are we playing?" message
        /// @return Message
        virtual inline const char* WhatAreWePlaying() final
        {
            return u8"Что будем слушать?";
        }

        /// @brief Generate search result message
        /// @param count Count of items in search
        /// @return Generated message
        virtual inline std::string SearchResult(unsigned int count) final
        {
            if (count == 0)
                return u8"Ничего не нашлось";
            return fmt::format(u8"{} результат{}", count, GetEnding(count));
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
                    return fmt::format(u8"Видео от {}, [{}]", video.author(), Utility::ToString(video.duration()));
                }
                case Youtube::Item::Type::Playlist:
                {
                    const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
                    return fmt::format(u8"Плейлист от {}, [{} видео]", playlist.author(), Utility::ToString(playlist.videoCount()));
                }
            }

            return "";
        }

        /// @brief Generate "I don't understand this timestamp" message
        /// @param timestamp Timestamp in question
        /// @return Generated message
        virtual inline std::string IncorrectTimestamp(const std::string& timestamp) final
        {
            return fmt::format(u8"Я не могу понять временную отметку \"{}\"", timestamp);
        }

        /// @brief Get "I'm not playing to seek" message
        /// @return Message
        virtual inline const char* NotPlayingToSeek() final
        {
            return u8"Я ничего не играю для перематывания";
        }

        /// @brief Generate "Seeking to timestamp" message
        /// @param timestamp Timestamp in question
        /// @return Generated message
        virtual inline std::string SeekingTo(const std::string& timestamp) final
        {
            return fmt::format(u8"Перематываю на `{}`", timestamp);
        }

        /// @brief Get "I don't recognise this slashcommand" message
        /// @return Message
        virtual inline const char* UnknownSlashcommand() final
        {
            return u8"Извини, я не узнаю эту команду";
        }

        /// @brief Get "I don't recognise this button" message
        /// @return Message
        virtual inline const char* UnknownButton() final
        {
            return u8"Извини, я не узнаю эту кнопку";
        }

        /// @brief Get "I don't recognise this option" message
        /// @return "I don't recognise this option" message
        virtual inline const char* UnknownOption() final
        {
            return u8"Извини, я не узнаю такой выбор";
        }

        /// @brief Generate "User used command" message
        /// @param username Name of user in question
        /// @param command The command used
        /// @return Generated message
        virtual inline std::string UsedCommand(const std::string& username, const std::string& command) final
        {
            return fmt::format(u8"{} использует /{}", username, command);
        }
    };
}

} // namespace kc
