#pragma once

namespace kc {

namespace Bot
{
    namespace Commands
    {
        namespace English
        {
            namespace Join
            {
                constexpr const char* Name = "join";
                constexpr const char* Description = "Join your voice channel";
            }

            namespace Leave
            {
                constexpr const char* Name = "leave";
                constexpr const char* Description = "Leave voice channel";
            }

            namespace Session
            {
                constexpr const char* Name = "session";
                constexpr const char* Description = "Show session info including playing video/playlist and queue";
            }

            namespace Play
            {
                constexpr const char* Name = "play";
                constexpr const char* Description = "Play YouTube video or playlist";

                namespace What
                {
                    constexpr const char* Name = "what";
                    constexpr const char* Description = "What do you want me to play?";
                }
            }

            namespace Pause
            {
                constexpr const char* Name = "pause";
                constexpr const char* Description = "Pause playing video";
            }

            namespace Resume
            {
                constexpr const char* Name = "resume";
                constexpr const char* Description = "Resume playing video";
            }

            namespace Seek
            {
                constexpr const char* Name = "seek";
                constexpr const char* Description = "Seek playing video";

                namespace TimestampChapter
                {
                    constexpr const char* Name = "timestamp-chapter";
                    constexpr const char* Description = "What timestamp or chapter do you want me to seek to?";
                }
            }

            namespace Shuffle
            {
                constexpr const char* Name = "shuffle";
                constexpr const char* Description = "Shuffle queue";
            }

            namespace Skip
            {
                constexpr const char* Name = "skip";

                namespace Video
                {
                    constexpr const char* Name = "video";
                    constexpr const char* Description = "Skip playing video";
                }

                namespace Playlist
                {
                    constexpr const char* Name = "playlist";
                    constexpr const char* Description = "Skip playing playlist";
                }
            }

            namespace Clear
            {
                constexpr const char* Name = "clear";
                constexpr const char* Description = "Clear queue";
            }

            namespace Stop
            {
                constexpr const char* Name = "stop";
                constexpr const char* Description = "Stop playing and clear queue";
            }

            namespace Settings
            {
                constexpr const char* Name = "settings";
                constexpr const char* Description = "Show settings of this guild";
            }

            namespace Stats
            {
                constexpr const char* Name = "stats";
                constexpr const char* Description = "Show stats of this guild";
            }

            namespace Set
            {
                constexpr const char* Name = "set";

                namespace Language
                {
                    constexpr const char* Name = "language";
                    constexpr const char* Description = "Set the language you want me to speak";

                    namespace Language
                    {
                        constexpr const char* Name = "language";
                        constexpr const char* Description = "What language to set?";
                        constexpr const char* Id = "en";
                        constexpr const char* Label = "English";
                    }
                }

                namespace Timeout
                {
                    constexpr const char* Name = "timeout";
                    constexpr const char* Description = "Set inactivity timeout duration";

                    namespace Duration
                    {
                        constexpr const char* Name = "duration";
                        constexpr const char* Description = "What inactivity timeout duration to set?";
                    }
                }
            }
        }

        namespace Russian
        {
            // Name of Discord locale these translated commands are going to be shown to
            constexpr const char* LocaleName = "ru";

            namespace Join
            {
                constexpr const char* Name = u8"зайти";
                constexpr const char* Description = u8"Зайти в твой голосовой канал";
            }

            namespace Leave
            {
                constexpr const char* Name = u8"выйти";
                constexpr const char* Description = u8"Выйти из голосового канала";
            }

            namespace Session
            {
                constexpr const char* Name = u8"сессия";
                constexpr const char* Description = u8"Показать информацию о сессии, включая играющее видео/плейлист и очередь";
            }

            namespace Play
            {
                constexpr const char* Name = u8"играть";
                constexpr const char* Description = u8"Играть видео или плейлист YouTube";

                namespace What
                {
                    constexpr const char* Name = u8"что";
                    constexpr const char* Description = u8"Что ты хочешь сыграть?";
                }
            }

            namespace Pause
            {
                constexpr const char* Name = u8"пауза";
                constexpr const char* Description = u8"Поставить воспроизведение видео на паузу";
            }

            namespace Resume
            {
                constexpr const char* Name = u8"продолжить";
                constexpr const char* Description = u8"Продолжить воспроизведение видео";
            }

            namespace Seek
            {
                constexpr const char* Name = u8"перемотать";
                constexpr const char* Description = u8"Перемотать играющее видео";

                namespace TimestampChapter
                {
                    constexpr const char* Name = u8"время-глава";
                    constexpr const char* Description = u8"На какое время или главу перемотать?";
                }
            }

            namespace Shuffle
            {
                constexpr const char* Name = u8"перемешать";
                constexpr const char* Description = u8"Перемешать очередь";
            }

            namespace Skip
            {
                constexpr const char* Name = u8"пропустить";

                namespace Video
                {
                    constexpr const char* Name = u8"видео";
                    constexpr const char* Description = u8"Пропустить играющее видео";
                }

                namespace Playlist
                {
                    constexpr const char* Name = u8"плейлист";
                    constexpr const char* Description = u8"Пропустить играющий плейлист";
                }
            }

            namespace Clear
            {
                constexpr const char* Name = u8"очистить";
                constexpr const char* Description = u8"Очистить очередь";
            }

            namespace Stop
            {
                constexpr const char* Name = u8"остановить";
                constexpr const char* Description = u8"Остановить воспроизведение и очистить очередь";
            }

            namespace Settings
            {
                constexpr const char* Name = u8"настройки";
                constexpr const char* Description = u8"Показать настройки этого сервера";
            }

            namespace Stats
            {
                constexpr const char* Name = u8"статистика";
                constexpr const char* Description = u8"Показать статистику этого сервера";
            }

            namespace Set
            {
                constexpr const char* Name = u8"установить";

                namespace Language
                {
                    constexpr const char* Name = u8"язык";
                    constexpr const char* Description = u8"Установить язык, на котором я буду разговаривать";

                    namespace Language
                    {
                        constexpr const char* Name = u8"язык";
                        constexpr const char* Description = u8"Какой язык установить?";
                        constexpr const char* Id = "ru";
                        constexpr const char* Label = u8"Русский";
                    }
                }

                namespace Timeout
                {
                    constexpr const char* Name = u8"тайм-аут";
                    constexpr const char* Description = u8"Установить продолжительность тайм-аута бездействия";

                    namespace Duration
                    {
                        constexpr const char* Name = u8"продолжительность";
                        constexpr const char* Description = u8"Какую продолжительность тайм-аута бездействия установить?";
                    }
                }
            }
        }
    }
}

} // namespace kc
