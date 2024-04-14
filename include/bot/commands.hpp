#pragma once

namespace kc {

namespace Bot
{
    namespace Commands
    {
        namespace Session
        {
            constexpr const char* Name = "session";
            constexpr const char* Description = "Show session info including playing video/playlist and queue";
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
                    constexpr const char* Description = "What inactivity timeout duration to set? (in minutes)";
                }
            }

            namespace ChangeStatus
            {
                constexpr const char* Name = "change-status";
                constexpr const char* Description = "Tell me if I'm allowed to change voice channel status when I'm playing";

                namespace Change
                {
                    constexpr const char* Name = "change";
                    constexpr const char* Description = "Am I allowed to modify voice channel status or not?";
                }
            }
        }
        
        /*
        *   The following commands control music player.
        *   Bot processes them only if issuing user is a listener (sitting with bot).
        */

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
            constexpr const char* Description = "Toggle player pause";
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

        namespace Russian
        {
            // Name of Discord locale these translated commands are going to be shown to
            // https://discord.com/developers/docs/reference#locales
            constexpr const char* LocaleName = "ru";

            namespace Session
            {
                constexpr const char* Name = Commands::Join::Name;
                constexpr const char* Description = u8"Показать информацию о сессии, включая играющее видео/плейлист и очередь";
            }

            namespace Settings
            {
                constexpr const char* Name = Commands::Settings::Name;
                constexpr const char* Description = u8"Показать настройки этого сервера";
            }

            namespace Stats
            {
                constexpr const char* Name = Commands::Stats::Name;
                constexpr const char* Description = u8"Показать статистику этого сервера";
            }

            namespace Set
            {
                constexpr const char* Name = Commands::Set::Name;

                namespace Language
                {
                    constexpr const char* Name = Commands::Set::Language::Name;
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
                    constexpr const char* Name = Commands::Set::Timeout::Name;
                    constexpr const char* Description = u8"Установить продолжительность тайм-аута бездействия";

                    namespace Duration
                    {
                        constexpr const char* Name = u8"продолжительность";
                        constexpr const char* Description = u8"Какую продолжительность тайм-аута бездействия установить? (в минутах)";
                    }
                }

                namespace ChangeStatus
                {
                    constexpr const char* Name = Commands::Set::ChangeStatus::Name;
                    constexpr const char* Description = u8"Скажи мне, можно ли мне менять статус голосового канала, когда я играю?";

                    namespace Change
                    {
                        constexpr const char* Name = u8"изменять";
                        constexpr const char* Description = u8"Можно ли мне изменять статус голосового канала или нет?";
                    }
                }
            }

            namespace Join
            {
                constexpr const char* Name = Commands::Join::Name;
                constexpr const char* Description = u8"Зайти в твой голосовой канал";
            }

            namespace Leave
            {
                constexpr const char* Name = Commands::Leave::Name;
                constexpr const char* Description = u8"Выйти из голосового канала";
            }

            namespace Play
            {
                constexpr const char* Name = Commands::Play::Name;
                constexpr const char* Description = u8"Играть видео или плейлист YouTube";

                namespace What
                {
                    constexpr const char* Name = u8"что";
                    constexpr const char* Description = u8"Что ты хочешь сыграть?";
                }
            }

            namespace Pause
            {
                constexpr const char* Name = Commands::Pause::Name;
                constexpr const char* Description = u8"Включить/выключить паузу плеера";
            }

            namespace Seek
            {
                constexpr const char* Name = Commands::Seek::Name;
                constexpr const char* Description = u8"Перемотать играющее видео";

                namespace TimestampChapter
                {
                    constexpr const char* Name = u8"время-глава";
                    constexpr const char* Description = u8"На какое время или главу перемотать?";
                }
            }

            namespace Shuffle
            {
                constexpr const char* Name = Commands::Shuffle::Name;
                constexpr const char* Description = u8"Перемешать очередь";
            }

            namespace Skip
            {
                constexpr const char* Name = Commands::Skip::Name;

                namespace Video
                {
                    constexpr const char* Name = Commands::Skip::Video::Name;
                    constexpr const char* Description = u8"Пропустить играющее видео";
                }

                namespace Playlist
                {
                    constexpr const char* Name = Commands::Skip::Playlist::Name;
                    constexpr const char* Description = u8"Пропустить играющий плейлист";
                }
            }

            namespace Clear
            {
                constexpr const char* Name = Commands::Clear::Name;
                constexpr const char* Description = u8"Очистить очередь";
            }

            namespace Stop
            {
                constexpr const char* Name = Commands::Stop::Name;
                constexpr const char* Description = u8"Остановить воспроизведение и очистить очередь";
            }
        }
    }
}

} // namespace kc
