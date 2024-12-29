#pragma once

// STL modules
#include <memory>
#include <vector>

// Library DPP
#include <dpp/dpp.h>

namespace kb {

namespace Bot
{
    namespace CommandsConst
    {
        namespace Help
        {
            constexpr const char* Name = "help";
            constexpr const char* Description = "Show help message";
        }

        namespace Session
        {
            constexpr const char* Name = "session";
            constexpr const char* Description = "Show current session information";
        }

        namespace Settings
        {
            constexpr const char* Name = "settings";
            constexpr const char* Description = "Show settings of this guild";
        }

        namespace Stats
        {
            constexpr const char* Name = "stats";
            constexpr const char* Description = "Show statistics of this guild";
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
                    constexpr const char* Description = "What language do you want me to speak?";
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
                constexpr const char* Description = "Tell me if I'm allowed to change voice channel status when I'm playing or not";

                namespace Change
                {
                    constexpr const char* Name = "change";
                    constexpr const char* Description = "Am I allowed to change voice channel status or not?";

                    namespace Yes
                    {
                        constexpr const char* Id = "yes";
                        constexpr const char* Label = "Yes";
                    }

                    namespace No
                    {
                        constexpr const char* Id = "no";
                        constexpr const char* Label = "No";
                    }
                }
            }
        }
        
        /*
        *   The following commands control music player.
        *   Bot processes them only if issuing user is a listener (sitting with bot in a voice channel).
        */

        namespace Join
        {
            constexpr const char* Name = "join";
            constexpr const char* Description = "Join your voice channel";
        }

        namespace Leave
        {
            constexpr const char* Name = "leave";
            constexpr const char* Description = "Leave voice channel that I'm currently sitting in";
        }

        namespace Play
        {
            constexpr const char* Name = "play";
            constexpr const char* Description = "Play YouTube video or playlist";

            namespace What
            {
                constexpr const char* Name = "what";
                constexpr const char* Description = "What video or playlist do you want me to play?";
            }
        }

        namespace Pause
        {
            constexpr const char* Name = "pause";
            constexpr const char* Description = "Pause playing video or resume playing if it's already paused";
        }

        namespace Seek
        {
            constexpr const char* Name = "seek";
            constexpr const char* Description = "Seek playing video to timestamp or chapter";

            namespace TimestampChapter
            {
                constexpr const char* Name = "timestamp-chapter";
                constexpr const char* Description = "What timestamp or chapter do you want me to seek to?";
            }
        }

        namespace Shuffle
        {
            constexpr const char* Name = "shuffle";
            constexpr const char* Description = "Shuffle videos and playlists in queue";
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
            constexpr const char* Description = "Stop playing video/playlist and clear queue";
        }

        namespace Russian
        {
            // Name of Discord locale these translated commands are going to be shown to
            // https://discord.com/developers/docs/reference#locales
            constexpr const char* LocaleName = "ru";

            namespace Help
            {
                constexpr const char* Name = CommandsConst::Help::Name;
                constexpr const char* Description = u8"Показать справочное сообщение";
            }

            namespace Session
            {
                constexpr const char* Name = CommandsConst::Session::Name;
                constexpr const char* Description = u8"Показать информацию о текущей сессии";
            }

            namespace Settings
            {
                constexpr const char* Name = CommandsConst::Settings::Name;
                constexpr const char* Description = u8"Показать настройки этого сервера";
            }

            namespace Stats
            {
                constexpr const char* Name = CommandsConst::Stats::Name;
                constexpr const char* Description = u8"Показать статистику этого сервера";
            }

            namespace Set
            {
                constexpr const char* Name = CommandsConst::Set::Name;

                namespace Language
                {
                    constexpr const char* Name = CommandsConst::Set::Language::Name;
                    constexpr const char* Description = u8"Установить язык, на котором я буду разговаривать";

                    namespace Language
                    {
                        constexpr const char* Name = u8"язык";
                        constexpr const char* Description = u8"На каком языке мне говорить?";
                        constexpr const char* Id = "ru";
                        constexpr const char* Label = u8"Русский";
                    }
                }

                namespace Timeout
                {
                    constexpr const char* Name = CommandsConst::Set::Timeout::Name;
                    constexpr const char* Description = u8"Установить продолжительность тайм-аута бездействия";

                    namespace Duration
                    {
                        constexpr const char* Name = u8"продолжительность";
                        constexpr const char* Description = u8"Какую продолжительность тайм-аута бездействия установить? (в минутах)";
                    }
                }

                namespace ChangeStatus
                {
                    constexpr const char* Name = CommandsConst::Set::ChangeStatus::Name;
                    constexpr const char* Description = u8"Скажи мне, можно ли мне менять статус голосового канала, когда я играю, или нет?";

                    namespace Change
                    {
                        constexpr const char* Name = u8"изменять";
                        constexpr const char* Description = u8"Можно ли мне изменять статус голосового канала или нет?";

                        namespace Yes
                        {
                            constexpr const char* Label = u8"Да";
                        }

                        namespace No
                        {
                            constexpr const char* Label = u8"Нет";
                        }
                    }
                }
            }

            namespace Join
            {
                constexpr const char* Name = CommandsConst::Join::Name;
                constexpr const char* Description = u8"Зайти в твой голосовой канал";
            }

            namespace Leave
            {
                constexpr const char* Name = CommandsConst::Leave::Name;
                constexpr const char* Description = u8"Выйти из голосового канала, в котором я сейчас сижу";
            }

            namespace Play
            {
                constexpr const char* Name = CommandsConst::Play::Name;
                constexpr const char* Description = u8"Сыграть видео или плейлист с YouTube";

                namespace What
                {
                    constexpr const char* Name = u8"что";
                    constexpr const char* Description = u8"Какое видео или плейлист ты хочешь сыграть?";
                }
            }

            namespace Pause
            {
                constexpr const char* Name = CommandsConst::Pause::Name;
                constexpr const char* Description = u8"Поставить играющее видео на паузу или снять с неё, если оно уже стоит на паузе";
            }

            namespace Seek
            {
                constexpr const char* Name = CommandsConst::Seek::Name;
                constexpr const char* Description = u8"Перемотать играющее видео на время или главу";

                namespace TimestampChapter
                {
                    constexpr const char* Name = u8"время-глава";
                    constexpr const char* Description = u8"На какое время или главу перемотать?";
                }
            }

            namespace Shuffle
            {
                constexpr const char* Name = CommandsConst::Shuffle::Name;
                constexpr const char* Description = u8"Перемешать видео и плейлисты в очереди";
            }

            namespace Skip
            {
                constexpr const char* Name = CommandsConst::Skip::Name;

                namespace Video
                {
                    constexpr const char* Name = CommandsConst::Skip::Video::Name;
                    constexpr const char* Description = u8"Пропустить играющее видео";
                }

                namespace Playlist
                {
                    constexpr const char* Name = CommandsConst::Skip::Playlist::Name;
                    constexpr const char* Description = u8"Пропустить играющий плейлист";
                }
            }

            namespace Clear
            {
                constexpr const char* Name = CommandsConst::Clear::Name;
                constexpr const char* Description = u8"Очистить очередь";
            }

            namespace Stop
            {
                constexpr const char* Name = CommandsConst::Stop::Name;
                constexpr const char* Description = u8"Остановить воспроизведение играющего видео/плейлиста и очистить очередь";
            }
        }
    }

    class Commands
    {
    public:
        // Singleton instance
        static const std::unique_ptr<Commands> Instance;

    public:
        /// @brief Get commands to register
        /// @param botId Bot user ID
        /// @return Commands to register
        static std::vector<dpp::slashcommand> GetCommands(dpp::snowflake botId);

    private:
        bool m_parsed;
        dpp::slashcommand m_help;
        dpp::slashcommand m_session;
        dpp::slashcommand m_settings;
        dpp::slashcommand m_stats;
        dpp::slashcommand m_set;
        dpp::slashcommand m_join;
        dpp::slashcommand m_leave;
        dpp::slashcommand m_play;
        dpp::slashcommand m_pause;
        dpp::slashcommand m_seek;
        dpp::slashcommand m_shuffle;
        dpp::slashcommand m_skip;
        dpp::slashcommand m_clear;
        dpp::slashcommand m_stop;

    private:
        Commands();

    public:
        /// @brief Parse bot commands
        /// @param commands Commands to parse
        /// @throw std::invalid_argument if unknown command is encountered
        void parse(const dpp::slashcommand_map& commands);

    public:
        /// @brief Check if commands are parsed
        /// @return True if commands are parsed
        inline operator bool() const
        {
            return m_parsed;
        }

        /// @brief Get /help command
        /// @return /help command
        inline const dpp::slashcommand& help() const
        {
            return m_help;
        }

        /// @brief Get /session command
        /// @return /session command
        inline const dpp::slashcommand& session() const
        {
            return m_session;
        }

        /// @brief Get /settings command
        /// @return /settings command
        inline const dpp::slashcommand& settings() const
        {
            return m_settings;
        }

        /// @brief Get /stats command
        /// @return /stats command
        inline const dpp::slashcommand& stats() const
        {
            return m_stats;
        }

        /// @brief Get /set command
        /// @return /set command
        inline const dpp::slashcommand& set() const
        {
            return m_set;
        }

        /// @brief Get /join command
        /// @return /join command
        inline const dpp::slashcommand& join() const
        {
            return m_join;
        }

        /// @brief Get /leave command
        /// @return /leave command
        inline const dpp::slashcommand& leave() const
        {
            return m_leave;
        }

        /// @brief Get /play command
        /// @return /play command
        inline const dpp::slashcommand& play() const
        {
            return m_play;
        }

        /// @brief Get /pause command
        /// @return /pause command
        inline const dpp::slashcommand& pause() const
        {
            return m_pause;
        }

        /// @brief Get /seek command
        /// @return /seek command
        inline const dpp::slashcommand& seek() const
        {
            return m_seek;
        }

        /// @brief Get /shuffle command
        /// @return /shuffle command
        inline const dpp::slashcommand& shuffle() const
        {
            return m_shuffle;
        }

        /// @brief Get /skip command
        /// @return /skip command
        inline const dpp::slashcommand& skip() const
        {
            return m_skip;
        }

        /// @brief Get /clear command
        /// @return /clear command
        inline const dpp::slashcommand& clear() const
        {
            return m_clear;
        }

        /// @brief Get /stop command
        /// @return /stop command
        inline const dpp::slashcommand& stop() const
        {
            return m_stop;
        }
    };
}

} // namespace kb
