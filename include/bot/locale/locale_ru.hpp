#pragma once

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale.hpp"
#include "bot/commands.hpp"

namespace kc {

namespace Bot
{
    class LocaleRu : public Locale
    {
    public:
        static constexpr Locale::Type Type = Locale::Type::Russian;
        static constexpr const char* Name = "ru";
        static constexpr const char* LongName = u8"Русский";

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
                    return u8"а";
                default:
                    return u8"ов";
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
            strings.ifYouNeedAnyHelp = u8"**Если тебе нужна помощь о том, как я играю музыку, используй {}.**";
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
            strings.hereAreAllOfMyCommands = u8"Вот все мои команды";
            strings.differentLanguagesHint = {
                ">>> ***I can speak different languages!***\n"
                "Now it's **`Russian`**.\n"
                "Use {} if you need to change it."
            };
            strings.helpDescription = u8"Показать это справочное сообщение.";
            strings.sessionDescription = u8"Показать информацию о текущей сессии: играющее видео и его главы, играющий плейлист и его видео, видео/плейлисты в очереди и прочее.";
            strings.sessionFaq = {
                u8">>> ***Что такое сессия?***\n"
                u8"Сессия - это такое состояние, когда я сижу в голосовом канале. Она начинается, когда я захожу, и заканчивается, когда я выхожу из него."
            };
            strings.settingsDescription = u8"Показать настройки этого сервера: язык, продолжительность тайм-аута бездействия и прочее.";
            strings.statsDescription = u8"Показать статистику этого сервера: количество отправленных команд, нажатых кнопок, сессий, проигранных треков и прочее.";
            strings.setDescription = u8"Установить настройки этого серера: язык, продолжительность тайм-аута бездействия, а также должен ли я менять статус голосового канала или нет.";
            strings.setFaq = {
                u8">>> ***Что такое тайм-аут бездействия?***\n"
                u8"Рано или поздно в очереди закончатся видео/плейлисты для воспроизведения, и я буду сидеть и ничего не делать. "
                u8"Эта настройка устанавливает количество времени, которое должно пройти перед тем, как я покину голосовой канал, если ничего не делаю.\n"
                u8"***Что такое статус голосового канала?***\n"
                u8"Это надпись чуть ниже названия канала. Для меня она - это самый простой и информативный способ сообщать всем, что сейчас играет. "
                u8"Его можно запретить, если на этом сервере статус голосового канала используется для других целей и ты не хочешь, чтобы я его менял."
            };
            strings.joinDescription = u8"Зайти в твой голосовой канал. Не забывай, что сперва ты сам должен зайти в него.";
            strings.leaveDescription = u8"Выйти из голосового канала, в котором я сейчас сижу.";
            strings.playDescription = u8"Проиграть видео или плейлист с YouTube. Твой запрос будет добавлен в очередь, если сейчас уже что-то играет.";
            strings.playFaq = {
                u8">>> ***Что и как может быть проиграно?***\n"
                u8"Почти всё что угодно может быть проиграно, даже YouTube #Shorts.\n"
                "*Однако, к сожалению, я пока не умею проигрывать стримы и премьеры.*\n"
                u8"Смело присылай ссылку на видео/плейлист, который ты хочешь проиграть, или начинай вводить его название, чтобы я поискал YouTube и помог тебе, показывая предложения."
            };
            strings.pauseDescription = u8"Поставить играющее видео на паузу или снять с неё, если оно уже стоит на паузе.";
            strings.seekDescription = u8"Перемотать играющее видео на временную метку или главу.";
            strings.seekFaq = {
                u8">>> ***Как перемотать на временную метку?***\n"
                u8"Достаточно просто написать временную метку в секундах (`50`, `743`, `9965`) или через двоеточия (`50`, `12:23`, `2:46:05`).\n"
                u8"*Если написанное не будет похоже на временную метку, то я интерпретирую его как название главы.*\n"
                u8"***Как перемотать на главу?***\n"
                u8"Такая перемотка возможна только если у видео есть главы. Начинай вводить название желаемой главы, чтобы я отфильтровал их и показал подходящие.\n"
                u8"*На пустое поле я покажу список всех глав, но из-за ограничения Discord их там будет не больше 25ти.*"
            };
            strings.shuffleDescription = u8"Перемешать видео/плейлисты в очереди. Не перемешывает видео в играющем плейлисте.";
            strings.skipDescription = u8"Пропустить играющее видео или весь плейлист.";
            strings.clearDescription = u8"Очистить видео/плейлисты в очереди. Не очищает играющее видео/плейлист.";
            strings.stopDescription = u8"Перестать играть играющее видео/плейлист и очистить очередь.";
            return HelpMessage(strings);
        }

        /// @brief Create "I am not sitting in any voice channel" message
        /// @return Ephemeral message
        virtual inline dpp::message botNotInVoiceChannel()
        {
            return ProblemMessage(u8"Я не сижу в голосовом канале");
        }

        /// @brief Create session message
        /// @param session Player session
        /// @return Ephemeral message
        virtual inline dpp::message session(const Session& session)
        {
            SessionStrings strings = {};
            strings.prettyQuiet = u8"Здесь довольно тихо";
            strings.nothingIsPlaying = u8"Ничего не играет. Добавь что-нибудь в очередь!";
            strings.videoInfo = {
                u8"от {}, [{}]\n"
                u8"{}, {} просмотр{}"
            };
            strings.requestedBy = u8"Запросил **{}**";
            strings.lastVideoPlaylistInfo = {
                u8"Плейлист от {}\n"
                u8"Играет последнее видео"
            };
            strings.playlistInfo = {
                u8"Плейлист от {}\n"
                u8"Осталось {} видео{:.0}"
            };
            strings.morePlaylistVideos = u8"... Ещё {} видео{:.0}";
            strings.playlistVideoInfo = u8"Видео от {}, [{}]";
            strings.playlistLivestreamInfo = {
                u8"Стрим от {}\n"
                u8"> Будет пропущен, потому что я не умею их играть!"
            };
            strings.playlistPremiereInfo = {
                u8"Премьера от {}\n"
                u8"> Будет пропущена, потому что я не умею их играть!"
            };
            strings.queueIsEmpty = u8"Очередь пуста";
            strings.queueInfo = u8"Очередь: {} видео/плейлист{}";
            strings.moreQueueItems = u8"... Ещё {} видео/плейлист{}";
            strings.queueVideoInfo = u8"Видео от {}, [{}]";
            strings.queuePlaylistInfo = u8"Плейлист от {}, [{} видео{:.0}]";
            return SessionMessage(strings, &Cardinal, session);
        }

        /// @brief Create guild settings message
        /// @param settings Guild settings
        /// @return Normal message
        virtual inline dpp::message settings(const Settings& settings)
        {
            SettingsStrings strings = {};
            strings.hereAreTheSettings = u8"Вот настройки этого сервера";
            strings.language = u8"Язык";
            strings.timeoutDuration = u8"Продолжительность тайм-аута";
            strings.changeStatus = u8"Разрешено менять статус голосового канала";
            strings.yes = u8"Да";
            strings.no = u8"Нет";
            return SettingsMessage(strings, settings);
        }

        /// @brief Create guild stats message
        /// @param stats Guild stats
        /// @return Normal message
        virtual inline dpp::message stats(const Stats& stats)
        {
            StatsStrings strings = {};
            strings.hereAreTheStats = u8"Вот статистика этого сервера";
            strings.interactionsProcessed = u8"Команд отправлено и кнопок нажато";
            strings.sessionsCount = u8"Количество сессий";
            strings.tracksPlayed = u8"Треков проиграно";
            strings.timesKicked = u8"Сколько раз я был кикнут";
            strings.timesMoved = u8"Сколько раз я был передвинут";
            return StatsMessage(strings, stats);
        }

        /// @brief Create "Sorry, I'm already taken!" message
        /// @return Ephemeral message
        virtual inline dpp::message alreadyTaken()
        {
            return ProblemMessage(u8"Извини, я уже занят!");
        }

        /// @brief Randomly create "So be it" or "Whatever you want" message
        /// @return Normal message
        virtual inline dpp::message soBeIt()
        {
            switch (Utility::RandomNumber(0, 4))
            {
                default:
                    return SuccessMessage(u8"Да будет так");
                case 1:
                    return SuccessMessage(u8"Как хочешь");
                case 2:
                    return SuccessMessage(u8"Хорошо");
                case 3:
                    return SuccessMessage(u8"Внимательно запоминаю");
                case 4:
                    return SuccessMessage(u8"Ок");
            }
        }

        /// @brief Create "Inactivity timeout duration must be greater than zero" message
        /// @return Ephemeral message
        virtual inline dpp::message badTimeoutDuration()
        {
            return ProblemMessage(u8"Продолжительность тайм-аута бездействия должна быть больше нуля");
        }

        /// @brief Create "Sorry, I'm not ready to sit and do nothing for more than two hours" message
        /// @return Ephemeral message
        virtual inline dpp::message longTimeoutDuration()
        {
            return ProblemMessage(u8"Уж извини, я не готов сидеть и ничего не делать больше двух часов");
        }

        /// @brief Create "Inactivity timeout duration is set to <duration>" message
        /// @param timeoutMinutes Duration of timeout in minutes
        /// @return Ephemeral message
        virtual inline dpp::message timeoutDurationSet(uint64_t timeoutMinutes)
        {
            return SuccessMessage(fmt::format(
                u8"Продолжительность тайм-аута бездействия установлена на `{}`",
                Utility::NiceString(pt::time_duration(0, timeoutMinutes, 0))
            ));
        }

        /// @brief Create "Something went wrong, I don't recognize this command" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownCommand()
        {
            return ErrorMessage(u8"Что-то пошло не так, я не узнаю эту команду");
        }

        /// @brief Create "Joining <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Normal message
        virtual inline dpp::message joining(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format(u8"Захожу в <#{}>", static_cast<uint64_t>(channelId)));
        }

        /// @brief Create "Already joined <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Ephemeral message
        virtual inline dpp::message alreadyJoined(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format(u8"Уже сижу в <#{}>", static_cast<uint64_t>(channelId))).set_flags(dpp::m_ephemeral);
        }

        /// @brief Create "I'm already sitting in another voice channel" message
        /// @param channelId ID of voice channel in question
        /// @return Ephemeral message
        virtual inline dpp::message cantJoin()
        {
            return ProblemMessage(u8"Я уже сижу в другом голосовом канале");
        }

        /// @brief Create "Join a voice channel first" message
        /// @return Ephemeral message
        virtual inline dpp::message userNotInVoiceChannel()
        {
            return ProblemMessage(u8"Сначала зайди в голосовой канал");
        }

        /// @brief Create "Left <voicechannel>" message
        /// @param channelId ID of voice channel in question
        /// @return Normal message
        virtual inline dpp::message left(dpp::snowflake channelId)
        {
            return SuccessMessage(fmt::format(u8"Вышел из <#{}>", static_cast<uint64_t>(channelId)));
        }

        /// @brief Create ambiguous play message
        /// @param videoId ID of video to play
        /// @param playlistId ID of playlist to play
        /// @return Ephemeral message
        virtual inline dpp::message ambiguousPlay(const std::string& videoId, const std::string& playlistId)
        {
            AmbigousPlayStrings strings = {};
            strings.playPlaylistOrVideo = u8"Проиграть весь плейлист или только это видео?";
            strings.playVideo = u8"Проиграть видео";
            strings.playPlaylist = u8"Проиграть плейлист";
            return AmbiguousPlayMessage(strings, videoId, playlistId);
        }

        /// @brief Create "Sorry, I can't play livestreams" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayLivestreams()
        {
            return ProblemMessage(u8"Извини, я не умею проигрывать стримы");
        }

        /// @brief Create "Sorry, I can't play premieres" message
        /// @return Ephemeral message
        virtual inline dpp::message cantPlayPremieres()
        {
            return ProblemMessage(u8"Извини, я не умею проигрывать премьеры");
        }

        /// @brief Create youtube error message
        /// @param error Error in question
        /// @return Ephemeral message
        virtual inline dpp::message youtubeError(const Youtube::YoutubeError& error)
        {
            switch (error.type())
            {
                case Youtube::YoutubeError::Type::LoginRequired:
                    return ProblemMessage(u8"У этого видео ограниченный доступ");
                case Youtube::YoutubeError::Type::Unplayable:
                {
                    dpp::message message = ProblemMessage(u8"Это видео заблокировано");
                    message.embeds[0].add_field("", {
                        u8">>> ***У тебя видео не заблокировано?***\n"
                        u8"Из-за разных причин, видео может быть доступно в одной стране, но заблокировано в другой. "
                        u8"Возможно, мы с тобой живём в разных странах."
                    });
                    return message;
                }
                case Youtube::YoutubeError::Type::YoutubeError:
                    return ProblemMessage(u8"Произошла ошибка на стороне YouTube");
                case Youtube::YoutubeError::Type::PlaylistError:
                    return ProblemMessage(u8"У этого плейлиста ограниченный доступ");
                default:
                    return ProblemMessage(u8"YouTube не дал мне проиграть это видео/плейлист");
            }
        }

        /// @brief Create local error message
        /// @param error Error in question
        /// @return Ephemeral message
        virtual inline dpp::message localError(const Youtube::LocalError& error)
        {
            switch (error.type())
            {
                case Youtube::LocalError::Type::PlaylistItemsNotSupported:
                    return ProblemMessage(u8"Извини, я не умею проигрывать ни одно видео в этом плейлисте");
                case Youtube::LocalError::Type::PlaylistNotSupported:
                    return ProblemMessage(u8"Извини, я не умею проигрывать плейлисты YouTube #Shorts");
                case Youtube::LocalError::Type::EmptyPlaylist:
                    return ProblemMessage(u8"Этот плейлист пуст");
                default:
                    return unknownError();
            }
        }

        /// @brief Create unknown error message
        /// @return Ephemeral message
        virtual inline dpp::message unknownError()
        {
            return ErrorMessage(u8"Извини, что-то пошло не так");
        }

        /// @brief Create "item added" message
        /// @param item Item in question
        /// @param paused Whether or not to display paused player warning
        /// @param requester User that added the item if needs to be shown in message
        /// @throw std::runtime_error if item type is unknown
        /// @return Normal message
        virtual inline dpp::message itemAdded(const Youtube::Item& item, bool paused, const std::optional<dpp::user>& requester = {})
        {
            ItemAddedStrings strings = {};
            strings.requestedBy = u8"Запрос от пользователя {}";
            strings.videoAdded = u8"Видео добавлено в очередь";
            strings.playlistAdded = u8"Плейлист добавлен в очередь";
            strings.playlistInfo = u8"{} [{} видео{:.0}]";
            strings.playAgain = u8"Ещё раз";
            strings.related = u8"Похожие";
            strings.youtube = u8"YouTube";

            dpp::message message = ItemAddedMessage(strings, item, &Cardinal, requester);
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create search message
        /// @param results Search results
        /// @return Ephemeral message
        virtual inline dpp::message search(const Youtube::Results& results)
        {
            SearchStrings strings = {};
            strings.noResults = u8"Ничего не нашлось";
            strings.resultCount = u8"\"{}\": {} результат{}";
            strings.relatedResultCount = u8"{} похожих результат{}";
            strings.videoInfo = u8"Видео от {}, [{}]";
            strings.playlistInfo = u8"Плейлист от {}, [{} видео]";
            strings.whatAreWePlaying = u8"Что будем слушать?";
            return SearchMessage(strings, &Cardinal, results);
        }

        /// @brief Create "Nothing is playing" message
        /// @return Ephemeral message
        virtual inline dpp::message nothingIsPlaying()
        {
            return ProblemMessage(u8"Ничего не играет");
        }

        /// @brief Create "Paused <video>" or "Resumed <video>" message
        /// @param video Playing video
        /// @param paused Whether or not player is paused
        /// @return Normal message
        virtual inline dpp::message paused(const Youtube::Video& video, bool paused)
        {
            return SuccessMessage(fmt::format(
                (paused ? u8"*{}* поставлено на паузу" : u8"Продолжаю играть *{}*"),
                video.title()
            ));
        }

        /// @brief Create "Duration of video <video> is only <duration>!" message
        /// @param video Playing video
        /// @return Ephemeral message
        virtual inline dpp::message timestampOutOfBounds(const Youtube::Video& video)
        {
            return ProblemMessage(fmt::format(u8"Продолжительность видео *{}* - `{}`", video.title(), Utility::NiceString(video.duration())));
        }

        /// @brief Create "Seeking <video> to <timestamp>" message
        /// @param video Seeking video
        /// @param timestamp Seek timestamp
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const Youtube::Video& video, pt::time_duration timestamp, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format(u8"Перематываю *{}* на `{}`", video.title(), Utility::NiceString(timestamp)));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "Seeking to chapter <chapter>, <timestamp>" message
        /// @param video Seeking video
        /// @param chapter The chapter in question
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message seeking(const Youtube::Video& video, const Youtube::Video::Chapter& chapter, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format(
                u8"Перематываю *{}* на главу *{}: {}*, `{}`",
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
        virtual inline dpp::message noChapters(const Youtube::Video& video)
        {
            return ProblemMessage(fmt::format(u8"У видео *{}* нет глав", video.title()));
        }

        /// @brief Get "Video <video> doesn't have such chapter" message
        /// @param video The video in question
        /// @return Ephemeral message
        virtual inline dpp::message unknownChapter(const Youtube::Video& video)
        {
            return ProblemMessage(fmt::format(u8"У видео *{}* нет такой главы", video.title()));
        }
 
        /// @brief Create "Queue is empty" message
        /// @return Ephemeral message
        virtual inline dpp::message queueIsEmpty()
        {
            return ProblemMessage(u8"Очередь пуста");
        }

        /// @brief Create "There is only one item in queue" message
        /// @return Ephemeral message
        virtual inline dpp::message cantShuffle()
        {
            return ProblemMessage(u8"В очереди находится только одно видео/плейлист");
        }

        /// @brief Create "Shuffled <count> items" message
        /// @param count Count of shuffled items
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message shuffled(size_t count, bool paused)
        {
            dpp::message message = SuccessMessage(fmt::format(u8"Перемешано {} видео/плейлист{}", count, Cardinal(count)));
            if (paused)
                pausedWarning(message.embeds[0]);
            return message;
        }

        /// @brief Create "Skipped item <item>" message
        /// @param item Skipped item
        /// @param paused Whether or not to display paused player warning
        /// @return Normal message
        virtual inline dpp::message skipped(const Youtube::Item& item, bool paused)
        {
            dpp::message message;
            switch (item.type())
            {
                case Youtube::Item::Type::Video:
                    message = SuccessMessage(fmt::format(u8"Пропущено видео *{}*", std::get<Youtube::Video>(item).title()));
                    break;
                case Youtube::Item::Type::Playlist:
                    message = SuccessMessage(fmt::format(u8"Пропущен плейлист *{}*", std::get<Youtube::Playlist>(item).title()));
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
            return ProblemMessage(u8"Никакой плейлист сейчас не играет");
        }

        /// @brief Create "Queue is cleared" message
        /// @return Normal message
        virtual inline dpp::message cleared()
        {
            return SuccessMessage(u8"Очередь очищена");
        }

        /// @brief Create "Stopped playing and cleared queue" message
        /// @return Normal message
        virtual inline dpp::message stopped()
        {
            return SuccessMessage(u8"Перестал играть и очистил очередь");
        }

        /// @brief Add paused player warning to embed
        /// @param embed Embed to add paused player warning to
        virtual inline void pausedWarning(dpp::embed& embed)
        {
            embed.set_footer({ u8"Ничего не играет, потому что плеер стоит на паузе! Используй /pause, чтобы продолжить" });
        }

        /// @brief Create item autocomplete choice
        /// @param item Item to create autocomplete choice for
        /// @throw std::runtime_error if item type is unknown
        /// @return Item autocomplete choice
        virtual inline dpp::command_option_choice itemAutocomplete(const Youtube::Item& item)
        {
            ItemAutocompleteStrings strings = {};
            strings.videoDescription = u8" - видео от {}, [{}]";
            strings.livestreamDescription = u8" - стрим от {}";
            strings.premiereDescription = u8" - премьера от {}";
            strings.playlistDescription = u8" - плейлист от {}, [{} видео{:.0}]";
            return ItemAutocompleteChoice(strings, &Cardinal, item);
        }

        /// @brief Create "Something went wrong, I don't recognize this button" message
        /// @return Ephemeral message
        virtual inline dpp::message unknownButton()
        {
            return ErrorMessage(u8"Что-то пошло не так, я не узнаю эту кнопку");
        }

        /// @brief Create "Skipping <livestream> because I can't play livestreams" message
        /// @param livestream Skipped livestream
        /// @return Normal message
        virtual inline dpp::message livestreamSkipped(const Youtube::Video& livestream)
        {
            return ProblemMessage(fmt::format(u8"Пропускаю *{}*, потому что я не умею играть стримы", livestream.title()));
        }

        /// @brief Create "Skipping <premiere> because I can't play premieres" message
        /// @param Skipped premiere
        /// @return Normal message
        virtual inline dpp::message premiereSkipped(const Youtube::Video& premiere)
        {
            return ProblemMessage(fmt::format(u8"Пропускаю *{}*, потому что я не умею играть премьеры", premiere.title()));
        }

        /// @brief Create "Something went wrong, I couldn't play <video>" message
        /// @param video The video in question
        /// @return Normal message
        virtual inline dpp::message playError(const Youtube::Video& video)
        {
            return ErrorMessage(fmt::format(u8"Что-то пошло не так: у меня не получилось проиграть *{}*", video.title())).set_flags(0);
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
            strings.sessionInfo = u8"Сессия #{1} пользователя {0} закончилась";
            strings.userRequested = u8"Пользователь попросил меня выйти";
            strings.timeout = u8"Я ничего не делал";
            strings.timeoutCanBeChanged = u8"Продолжительность тайм-аута может быть изменена с помощью {}";
            strings.everybodyLeft = u8"Я остался один в голосовом канале";
            strings.kicked = u8"Кто-то меня кикнул!";
            strings.voiceStatusNotCleared = {
                u8"К сожалению, я не смог очистить статус голосового канала.\n"
                u8"Discord позволяет делать это только тогда, когда я сижу в нём."
            };
            strings.moved = u8"Кто-то меня передвинул!";
            strings.sessionStats = u8"Статистика сессии";
            strings.lasted = u8"Продлилась";
            strings.tracksPlayed = u8"Треков проиграно";
            return EndMessage(strings, settings, reason, session);
        }

        /// @brief Create "Not playing" string
        /// @return "Not playing" string
        virtual inline const char* notPlaying()
        {
            return u8"Ничего не играет";
        }

        /// @brief Create "[Paused]" string
        /// @return "[Paused]" string
        virtual inline const char* paused()
        {
            return u8"[Пауза]";
        }
    };
}

} // namespace kc
