#pragma once

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale.hpp"
#include "core/utility.hpp"

namespace kb {

namespace Bot
{
    class LocaleRu : public Locale
    {
    public:
        static constexpr Locale::Type Type = Locale::Type::Russian;
        static constexpr const char* Name = "ru";
        static constexpr const char* LongName = "Русский";

    public:
        /// @brief Get number's cardinal ending
        /// @param number The number in question
        /// @return Number's cardinal ending
        static inline const char* Cardinal(size_t number)
        {
            switch (number % 10)
            {
                case 1:
                    return "";
                case 2:
                case 3:
                case 4:
                    return "а";
                default:
                    return "ов";
            }
        }

    public:
        /// @brief Get locale type
        /// @return Locale type
        virtual inline Locale::Type type()
        {
            return Type;
        }

        /// @brief Get locale name
        /// @return Locale name
        virtual inline const char* name()
        {
            return Name;
        }

        /// @brief Get long locale name
        /// @return Long locale name
        virtual inline const char* longName()
        {
            return LongName;
        }

        /// @brief Create mention reply message
        /// @return Mention reply message
        virtual inline dpp::message mention()
        {
            MentionReplyStrings strings = {};
            strings.ifYouNeedAnyHelp = "**Если тебе нужна помощь о том, как я играю музыку, используй {}.**";
            strings.differentLanguagesHint = {
                ">>> ***I can speak different languages!***\n"
                "Now it's **`Russian`**.\n"
                "Use {} if you need to change it."
            };
            return MentionReplyMessage(strings);
        }

        /// @brief Create help message
        /// @return Ephemeral message
        virtual inline dpp::message help()
        {
            HelpStrings strings = {};
            strings.hereAreAllOfMyCommands = "Вот все мои команды";
            strings.differentLanguagesHint = {
                ">>> ***I can speak different languages!***\n"
                "Now it's **`Russian`**.\n"
                "Use {} if you need to change it."
            };
            strings.helpDescription = "Показать это справочное сообщение.";
            strings.sessionDescription = "Показать информацию о текущей сессии: играющее видео и его главы, играющий плейлист и его видео, видео/плейлисты в очереди и прочее.";
            strings.sessionFaq = {
                ">>> ***Что такое сессия?***\n"
                "Сессия - это такое состояние, когда я сижу в голосовом канале. Она начинается, когда я захожу, и заканчивается, когда я выхожу из него."
            };
            strings.settingsDescription = "Показать настройки этого сервера: язык, продолжительность тайм-аута бездействия и прочее.";
            strings.statsDescription = "Показать статистику этого сервера: количество отправленных команд, нажатых кнопок, проведённых сессий, проигранных треков и прочее.";
            strings.setDescription = "Установить настройки этого серера: язык, продолжительность тайм-аута бездействия, а также должен ли я менять статус голосового канала или нет.";
            strings.setFaq = {
                ">>> ***Что такое тайм-аут бездействия?***\n"
                "Рано или поздно в очереди закончатся видео/плейлисты для воспроизведения, и я буду сидеть и ничего не делать. "
                "Эта настройка устанавливает количество времени, которое должно пройти перед тем, как я покину голосовой канал, если ничего не делаю.\n"
                "***Что такое статус голосового канала?***\n"
                "Это надпись чуть ниже названия канала. Для меня она - это самый простой и информативный способ сообщать всем, что сейчас играет. "
                "Его можно запретить, если на этом сервере статус голосового канала используется для других целей и ты не хочешь, чтобы я его менял."
            };
            strings.joinDescription = "Зайти в твой голосовой канал. Не забывай, что сперва ты сам должен зайти в него.";
            strings.leaveDescription = "Выйти из голосового канала, в котором я сейчас сижу.";
            strings.playDescription = "Проиграть видео или плейлист с YouTube. Твой запрос будет добавлен в очередь, если сейчас уже что-то играет.";
            strings.playFaq = {
                ">>> ***Что и как может быть проиграно?***\n"
                "Почти всё что угодно может быть проиграно, даже YouTube #Shorts.\n"
                "*Однако, к сожалению, я пока не умею проигрывать стримы и премьеры.*\n"
                "Смело присылай ссылку на видео/плейлист, который ты хочешь проиграть, или начинай вводить его название, чтобы я поискал YouTube и помог тебе, показывая предложения."
            };
            strings.pauseDescription = "Поставить играющее видео на паузу или снять с неё, если оно уже стоит на паузе.";
            strings.seekDescription = "Перемотать играющее видео на временную метку или главу.";
            strings.seekFaq = {
                ">>> ***Как перемотать на временную метку?***\n"
                "Достаточно просто написать временную метку в секундах (`50`, `743`, `9965`) или через двоеточия (`50`, `12:23`, `2:46:05`).\n"
                "*Если написанное не будет похоже на временную метку, то я интерпретирую его как название главы.*\n"
                "***Как перемотать на главу?***\n"
                "Такая перемотка возможна только если у видео есть главы. Начинай вводить название желаемой главы, чтобы я отфильтровал их и показал подходящие.\n"
                "*На пустое поле я покажу список всех глав, но из-за ограничения Discord их там будет не больше 25ти.*"
            };
            strings.shuffleDescription = "Перемешать видео/плейлисты в очереди. Не перемешывает видео в играющем плейлисте.";
            strings.skipDescription = "Пропустить играющее видео или весь плейлист.";
            strings.clearDescription = "Очистить видео/плейлисты в очереди. Не очищает играющее видео/плейлист.";
            strings.stopDescription = "Перестать играть играющее видео/плейлист и очистить очередь.";
            return HelpMessage(strings);
        }

        /// @brief Create "I am not sitting in any voice channel" message
        /// @return Ephemeral message
        virtual inline dpp::message botNotInVoiceChannel()
        {
            return ProblemMessage("Я не сижу в голосовом канале");
        }

        /// @brief Create session message
        /// @param session Player session
        /// @return Ephemeral message
        virtual inline dpp::message session(const Session& session)
        {
            SessionStrings strings = {};
            strings.infoFooter = "Сессия продлилась {} | Треков проиграно: {}";
            strings.prettyQuiet = "Здесь довольно тихо";
            strings.nothingIsPlaying = "Ничего не играет. Добавь что-нибудь в очередь!";
            strings.video = "Видео";
            strings.chapter = "Глава";
            strings.videoInfo = "от {}, [{}]";
            strings.requestedBy = "Запрос от **{}**";
            strings.lastVideoPlaylistInfo = {
                "Плейлист от {}\n"
                "Играет последнее видео"
            };
            strings.playlistInfo = {
                "Плейлист от {}\n"
                "Осталось {} видео{:.0}"
            };
            strings.morePlaylistVideos = "... Ещё {} видео{:.0}";
            strings.playlistVideoInfo = "Видео от {}, [{}]";
            strings.playlistLivestreamInfo = {
                "Стрим от {}\n"
                "> Будет пропущен, потому что я не умею их играть!"
            };
            strings.playlistPremiereInfo = {
                "Премьера от {}\n"
                "> Будет пропущена, потому что я не умею их играть!"
            };
            strings.queueIsEmpty = "Очередь пуста";
            strings.queueInfo = "Очередь: {} видео/плейлист{}";
            strings.moreQueueItems = "... Ещё {} видео/плейлист{}";
            strings.queueVideoInfo = "Видео от {}, [{}]";
            strings.queuePlaylistInfo = "Плейлист от {}, [{} видео{:.0}]";
            return SessionMessage(strings, &Cardinal, session);
        }

        /// @brief Create guild settings message
        /// @param settings Guild settings
        /// @return Normal message
        virtual inline dpp::message settings(const Settings& settings)
        {
            SettingsStrings strings = {};
            strings.hereAreTheSettings = "Вот настройки этого сервера";
            strings.language = "Язык";
            strings.timeoutDuration = "Продолжительность тайм-аута";
            strings.changeStatus = "Разрешено менять статус голосового канала";
            strings.yes = "Да";
            strings.no = "Нет";
            return SettingsMessage(strings, settings);
        }

        /// @brief Create guild stats message
        /// @param stats Guild stats
        /// @return Normal message
        virtual inline dpp::message stats(const Stats& stats)
        {
            StatsStrings strings = {};
            strings.hereAreTheStats = "Вот статистика этого сервера";
            strings.interactionsProcessed = "Команд отправлено и кнопок нажато";
            strings.sessionsConducted = "Сессий проведено";
            strings.tracksPlayed = "Треков проиграно";
            strings.timesKicked = "Сколько раз я был кикнут";
            strings.timesMoved = "Сколько раз я был передвинут";
            return StatsMessage(strings, stats);
        }

        /// @brief Create "Only users sitting with me can control the player" message
        /// @return Ephemeral message
        virtual inline dpp::message onlyUsersWithMeCanControlPlayer()
        {
            return ProblemMessage("Только сидящие со мной пользователи могут управлять плеером");
        }

        /// @brief Randomly create "So be it" or "Whatever you want" message
        /// @return Normal message
        virtual inline dpp::message soBeIt()
        {
            switch (Utility::RandomNumber(0, 4))
            {
                default:
                    return SuccessMessage("Да будет так");
                case 1:
                    return SuccessMessage("Как хочешь");
                case 2:
                    return SuccessMessage("Хорошо");
                case 3:
                    return SuccessMessage("Внимательно запоминаю");
                case 4:
                    return SuccessMessage("Ок");
            }
        }

        /// @brief Create "Inactivity timeout duration must be greater than zero" message
        /// @return Ephemeral message
        virtual inline dpp::message badTimeoutDuration()
        {
            return ProblemMessage("Продолжительность тайм-аута бездействия должна быть больше нуля");
        }

        /// @brief Create "Sorry, I'm not ready to sit and do nothing for more than two hours" message
        /// @return Ephemeral message
        virtual inline dpp::message longTimeoutDuration()
        {
            return ProblemMessage("Уж извини, я не готов сидеть и ничего не делать больше двух часов");
        }

        /// @brief Create "Inactivity timeout duration is set to <duration>" message
        /// @param timeoutMinutes Duration of timeout in minutes
        /// @return Ephemeral message
        virtual inline dpp::message timeoutDurationSet(uint64_t timeoutMinutes)
        {
            return SuccessMessage(fmt::format(
                "Продолжительность тайм-аута бездействия установлена на `{}`",
                Utility::NiceString(pt::time_duration(0, timeoutMinutes, 0))
            ));
        }

        /// @brief Create "Something went wrong, I don't recognize this command" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownCommand()
        {
            return ErrorMessage("Что-то пошло не так, я не узнаю эту команду");
        }

        /// @brief Create "Joining <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Normal message
        virtual inline dpp::message joining(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format("Захожу в <#{}>", static_cast<uint64_t>(channelId)));
        }

        /// @brief Create "Already joined <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Ephemeral message
        virtual inline dpp::message alreadyJoined(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format("Уже сижу в <#{}>", static_cast<uint64_t>(channelId))).set_flags(dpp::m_ephemeral);
        }

        /// @brief Create "I'm already sitting in another voice channel" message
        /// @param channelId ID of voice channel in question
        /// @return Ephemeral message
        virtual inline dpp::message cantJoin()
        {
            return ProblemMessage("Я уже сижу в другом голосовом канале");
        }

        /// @brief Create "Join a voice channel first" message
        /// @return Ephemeral message
        virtual inline dpp::message userNotInVoiceChannel()
        {
            return ProblemMessage("Сначала зайди в голосовой канал");
        }

        /// @brief Crate "I can't play in AFK channels!" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayInAfkChannels()
        {
            return ProblemMessage("Я не могу играть в АФК каналах!");
        }

        /// @brief Create "Left <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Normal message
        virtual inline dpp::message left(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format("Вышел из <#{}>", static_cast<uint64_t>(channelId)));
        }

        /// @brief Create ambiguous play message
        /// @param videoId ID of video to play
        /// @param playlistId ID of playlist to play
        /// @return Ephemeral message
        virtual inline dpp::message ambiguousPlay(const std::string& videoId, const std::string& playlistId)
        {
            AmbigousPlayStrings strings = {};
            strings.playPlaylistOrVideo = "Проиграть весь плейлист или только это видео?";
            strings.playVideo = "Проиграть видео";
            strings.playPlaylist = "Проиграть плейлист";
            return AmbiguousPlayMessage(strings, videoId, playlistId);
        }

        /// @brief Create "Sorry, I can't play livestreams" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayLivestreams()
        {
            return ProblemMessage("Извини, я не умею проигрывать стримы");
        }

        /// @brief Create "Sorry, I can't play premieres" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayPremieres()
        {
            return ProblemMessage("Извини, я не умею проигрывать премьеры");
        }

        virtual inline dpp::message cantPlayEmptyPlaylists() {
            return ProblemMessage("Этот плейлист пуст");
        }

        virtual inline dpp::message temporarilyUnsupported() {
            return GenericMessage(
                LocaleConst::Colors::TemporarilyUnsupported,
                LocaleConst::Emojis::TemporarilyUnsupported,
                "Извини, это пока что недоступно", true
            );
        }

        /// @brief Create youtube error message
        /// @param error Error in question
        /// @return Ephemeral message
        virtual inline dpp::message youtubeError(const ytcpp::YtError& error)
        {
            switch (error.type())
            {
                case ytcpp::YtError::Type::Private:
                    return ProblemMessage("У этого видео ограниченный доступ");
                case ytcpp::YtError::Type::Unplayable:
                    return ProblemMessage("Ошибка проигрывания");
                case ytcpp::YtError::Type::Unavailable:
                    return ProblemMessage("Это видео недоступно");
                default:
                    return ProblemMessage("YouTube не дал мне проиграть это видео/плейлист");
            }
        }

        /// @brief Create unknown error message
        /// @return Ephemeral message
        virtual inline dpp::message unknownError()
        {
            return ErrorMessage("Извини, что-то пошло не так");
        }

        /// @brief Create "item added" message
        /// @param item Item in question
        /// @param paused Whether or not to display paused player warning
        /// @param requester User that added the item if needs to be shown in message
        /// @throw std::runtime_error if item type is unknown
        /// @return Normal message
        virtual inline dpp::message itemAdded(const ytcpp::Item& item, bool paused, const std::optional<dpp::user>& requester = {})
        {
            ItemAddedStrings strings = {};
            strings.requestedBy = "Запрос от пользователя {}";
            strings.videoAdded = "Видео добавлено в очередь";
            strings.playlistAdded = "Плейлист добавлен в очередь";
            strings.playlistInfo = "{} [{} видео{:.0}]";
            strings.playAgain = "Ещё раз";
            strings.related = "Похожие";
            strings.youtube = "YouTube";

            dpp::message message = ItemAddedMessage(strings, item, &Cardinal, requester);
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create search message
        /// @param results Search results
        /// @return Ephemeral message
        virtual inline dpp::message search(const ytcpp::SearchResults& results)
        {
            SearchStrings strings = {};
            strings.noResults = "Ничего не нашлось";
            strings.resultCount = "\"{}\": {} результат{}";
            strings.relatedResultCount = "{} похожих результат{}";
            strings.videoInfo = "Видео от {}, [{}]";
            strings.playlistInfo = "Плейлист от {}, [{} видео]";
            strings.whatAreWePlaying = "Что будем слушать?";
            return SearchMessage(strings, &Cardinal, results);
        }

        /// @brief Create "Nothing is playing" message
        /// @return Ephemeral message
        virtual inline dpp::message nothingIsPlaying()
        {
            return ProblemMessage("Ничего не играет");
        }

        /// @brief Create "Paused <video>" or "Resumed <video>" message
        /// @param video Playing video
        /// @param paused Whether or not player is paused
        /// @return Normal message
        virtual inline dpp::message paused(const ytcpp::Video& video, bool paused)
        {
            return SuccessMessage(fmt::format(
                fmt::runtime(paused ? "*{}* поставлено на паузу" : "Продолжаю играть *{}*"),
                video.title()
            ));
        }

        /// @brief Create "Duration of video <video> is only <duration>!" message
        /// @param video Playing video
        /// @return Ephemeral message
        virtual inline dpp::message timestampOutOfBounds(const ytcpp::Video& video)
        {
            return ProblemMessage(fmt::format("Продолжительность видео *{}* - `{}`", video.title(), Utility::NiceString(video.duration())));
        }

        /// @brief Create "Seeking <video> to <timestamp>" message
        /// @param video Seeking video
        /// @param timestamp Seek timestamp
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const ytcpp::Video& video, pt::time_duration timestamp, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format("Перематываю *{}* на `{}`", video.title(), Utility::NiceString(timestamp)));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

/* Temporarily unsupported!
        /// @brief Create "Seeking to chapter <chapter>, <timestamp>" message
        /// @param video Seeking video
        /// @param chapter The chapter in question
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const ytcpp::Video& video, const ytcpp::Video::Chapter& chapter, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format(
                "Перематываю *{}* на главу *{}: {}*, `{}`",
                video.title(),
                chapter.number,
                chapter.name,
                Utility::NiceString(chapter.timestamp)
            ));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "Video <video> has no chapters" message
        /// @param video The video in question
        /// @return Ephemeral message
        virtual inline dpp::message noChapters(const ytcpp::Video& video)
        {
            return ProblemMessage(fmt::format("У видео *{}* нет глав", video.title()));
        }

        /// @brief Get "Video <video> doesn't have such chapter" message
        /// @param video The video in question
        /// @return Ephemeral message
        virtual inline dpp::message unknownChapter(const ytcpp::Video& video)
        {
            return ProblemMessage(fmt::format("У видео *{}* нет такой главы", video.title()));
        }
*/

        /// @brief Create "Queue is empty" message
        /// @return Ephemeral message
        virtual inline dpp::message queueIsEmpty()
        {
            return ProblemMessage("Очередь пуста");
        }

        /// @brief Create "There is only one item in queue" message
        /// @return Ephemeral message
        virtual inline dpp::message cantShuffle()
        {
            return ProblemMessage("В очереди находится только одно видео/плейлист");
        }

        /// @brief Create "Shuffled <count> items" message
        /// @param count Count of shuffled items
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message shuffled(size_t count, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format("Перемешано {} видео/плейлист{}", count, Cardinal(count)));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "Skipped item <item>" message
        /// @param item Skipped item
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message skipped(const ytcpp::Item& item, bool paused)
        {
            dpp::message message;
            switch (item.type())
            {
                case ytcpp::Item::Type::Video:
                    message = SuccessMessage(fmt::format("Пропущено видео *{}*", std::get<ytcpp::Video>(item).title()));
                    break;
                case ytcpp::Item::Type::Playlist:
                    message = SuccessMessage(fmt::format("Пропущен плейлист *{}*", std::get<ytcpp::Playlist>(item).title()));
                    break;
                default:
                    return unknownError();
            }

            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "No playlist is playing" message
        /// @return Ephemeral message
        virtual inline dpp::message noPlaylistIsPlaying()
        {
            return ProblemMessage("Никакой плейлист сейчас не играет");
        }

        /// @brief Create "Queue is cleared" message
        /// @return Normal message
        virtual inline dpp::message cleared()
        {
            return SuccessMessage("Очередь очищена");
        }

        /// @brief Create "Stopped playing and cleared queue" message
        /// @return Normal message
        virtual inline dpp::message stopped()
        {
            return SuccessMessage("Перестал играть и очистил очередь");
        }

        /// @brief Add paused player warning to embed
        /// @param embed Embed to add paused player warning to
        virtual inline void pausedWarning(dpp::embed& embed)
        {
            embed.set_footer({ "Ничего не играет, потому что плеер стоит на паузе! Используй /pause, чтобы продолжить" });
        }

        /// @brief Create "Sorry, this button is no longer supported: use slashcommands instead" messagee
        /// @return Ephemeral message
        virtual inline dpp::message unsupportedButton()
        {
            return ProblemMessage("Извини, эта кнопка больше не работает: вместо неё используй команды");
        }

        /// @brief Create "Something went wrong, I don't recognize this button" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownButton()
        {
            return ErrorMessage("Что-то пошло не так, я не узнаю эту кнопку");
        }

        /// @brief Create "Something went wrong, I couldn't play <video>" message
        /// @param video The video in question
        /// @return Normal message
        virtual inline dpp::message playError(const ytcpp::Video& video)
        {
            return ErrorMessage(fmt::format("Что-то пошло не так: у меня не получилось проиграть *{}*", video.title())).set_flags(0);
        }

        /// @brief Create "Something happened with my connection to Discord... Playing <video> from the start" message
        /// @param video The video in question
        /// @return Normal message
        virtual inline dpp::message reconnectedPlay(const ytcpp::Video& video)
        {
            return QuestionMessage(fmt::format(
                "Что-то произошло с моим подключением к Discord... Играю *{}* с начала",
                video.title()
            ));
        }

        /// @brief Create session end message
        /// @param settings Guild's settings
        /// @param reason Session end reason
        /// @param session Player session
        /// @throw std::runtime_error if reason is unknown
        /// @return Normal message
        virtual inline dpp::message sessionEnd(const Settings& settings, EndReason reason, Session session)
        {
            EndStrings strings = {};
            strings.sessionInfo = "Сессия #{1} пользователя {0} закончилась";
            strings.userRequested = "Пользователь попросил меня выйти";
            strings.timeout = "Я ничего не делал";
            strings.timeoutCanBeChanged = "Продолжительность тайм-аута может\nбыть изменена с помощью {}";
            strings.everybodyLeft = "Все вышли из голосового канала";
            strings.kicked = "Кто-то меня кикнул!";
            strings.voiceStatusNotCleared = {
                "К сожалению, я не смог очистить статус голосового канала.\n"
                "Discord позволяет делать это только тогда, когда я сижу в нём."
            };
            strings.moved = "Кто-то меня передвинул!";
            strings.sessionStats = "Статистика сессии";
            strings.lasted = "Продлилась";
            strings.tracksPlayed = "Треков проиграно";
            return EndMessage(strings, settings, reason, session);
        }

        /// @brief Create "Not playing" string
        /// @return "Not playing" string
        virtual inline const char* notPlaying()
        {
            return "Ничего не играет";
        }

        /// @brief Create "[Paused]" string
        /// @return "[Paused]" string
        virtual inline const char* paused()
        {
            return "[Пауза]";
        }

        /// @brief Create "Video" string
        /// @return "Video" string
        virtual inline const char* video()
        {
            return "Видео";
        }

        /// @brief Create "Chapter" string
        /// @return "Chapter" string
        virtual inline const char* chapter()
        {
            return "Глава";
        }
    };
}

} // namespace kb
