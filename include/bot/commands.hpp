#pragma once

namespace kc {

namespace Bot
{
    namespace Commands
    {
        namespace Join
        {
            constexpr const char* Name = "join";
            constexpr const char* Description = "Join your voice channel";
            constexpr const char* DescriptionRu = u8"Присоединиться к твоему голосовому каналу";
        }

        namespace Leave
        {
            constexpr const char* Name = "leave";
            constexpr const char* Description = "Leave voice channel";
            constexpr const char* DescriptionRu = u8"Покинуть голосовой канал";
        }

        namespace Session
        {
            constexpr const char* Name = "session";
            constexpr const char* Description = "Show playing video/playlist and queue";
            constexpr const char* DescriptionRu = u8"Показать играющее видео/плейлист и очередь";
        }

        namespace Play
        {
            constexpr const char* Name = "play";
            constexpr const char* Description = "Play YouTube video or playlist";
            constexpr const char* DescriptionRu = u8"Воспроизвести видео или плейлист YouTube";

            namespace What
            {
                constexpr const char* Name = "what";
                constexpr const char* Description = "What do you want me to play?";
                constexpr const char* NameRu = u8"что";
                constexpr const char* DescriptionRu = u8"Что ты хочешь воспроизвести?";
            }
        }

        namespace Pause
        {
            constexpr const char* Name = "pause";
            constexpr const char* Description = "Pause playing video";
            constexpr const char* DescriptionRu = u8"Поставить воспроизведение видео на паузу";
        }

        namespace Resume
        {
            constexpr const char* Name = "resume";
            constexpr const char* Description = "Resume playing video";
            constexpr const char* DescriptionRu = u8"Продолжить воспроизведение видео";
        }

        namespace Seek
        {
            constexpr const char* Name = "seek";
            constexpr const char* Description = "Seek playing video";
            constexpr const char* DescriptionRu = u8"Перемотать играющее видео";

            namespace TimestampChapter
            {
                constexpr const char* Name = "timestamp-chapter";
                constexpr const char* Description = "Timestamp or chapter name";
                constexpr const char* DescriptionRu = u8"Временная метка или название главы";
            }
        }

        namespace Shuffle
        {
            constexpr const char* Name = "shuffle";
            constexpr const char* Description = "Shuffle items in queue";
            constexpr const char* DescriptionRu = u8"Перемешать видео/плейлисты в очереди";
        }

        namespace Skip
        {
            constexpr const char* Name = "skip";

            namespace Video
            {
                constexpr const char* Name = "video";
                constexpr const char* Description = "Skip playing video";
                constexpr const char* DescriptionRu = u8"Пропустить играющее видео";
            }

            namespace Playlist
            {
                constexpr const char* Name = "playlist";
                constexpr const char* Description = "Skip playing playlist";
                constexpr const char* DescriptionRu = u8"Пропустить играющий плейлист";
            }
        }

        namespace Clear
        {
            constexpr const char* Name = "clear";
            constexpr const char* Description = "Clear queue";
            constexpr const char* DescriptionRu = u8"Очистить очередь";
        }

        namespace Stop
        {
            constexpr const char* Name = "stop";
            constexpr const char* Description = "Stop playing and clear queue";
            constexpr const char* DescriptionRu = u8"Перестать играть и очистить очередь";
        }

        namespace Settings
        {
            constexpr const char* Name = "settings";
            constexpr const char* Description = "Show this guild's settings";
            constexpr const char* DescriptionRu = u8"Показать настройки этого сервера";
        }

        namespace Stats
        {
            constexpr const char* Name = "stats";
            constexpr const char* Description = "Show this guild's stats";
            constexpr const char* DescriptionRu = u8"Показать статистику этого сервера";
        }

        namespace Set
        {
            constexpr const char* Name = "set";

            namespace Language
            {
                constexpr const char* Name = "language";
                constexpr const char* Description = "Set language you want me to speak";
                constexpr const char* DescriptionRu = u8"Установить язык, на котором я буду разговаривать";

                namespace Language
                {
                    constexpr const char* Name = "language";
                    constexpr const char* Description = "The language to set";
                    constexpr const char* NameRu = "язык";
                    constexpr const char* DescriptionRu = "Язык, который нужно установить";

                    namespace English
                    {
                        constexpr const char* Name = "en";
                        constexpr const char* Label = "English";
                    }

                    namespace Russian
                    {
                        constexpr const char* Name = "ru";
                        constexpr const char* Label = u8"Русский";
                    }
                }
            }

            namespace Timeout
            {
                constexpr const char* Name = "timeout";
                constexpr const char* Description = "Set inactivity timeout duration";
                constexpr const char* DescriptionRu = u8"Установить длительность тайм-аута бездействия";

                namespace Duration
                {
                    constepxr const char* Name = "duration";
                    constexpr const char* Description = "The timeout duration to set";
                    constexpr const char* NameRu = u8"продолжительность";
                    constexpr const char* DescriptionRu = u8"Продолжительность тайм-аута, которую нужно установить";
                }
            }
        }
    }
}

} // namespace kc
