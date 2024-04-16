#include "bot/locale/locale.hpp"
#include "bot/locale/locale_en.hpp"
#include "bot/locale/locale_ru.hpp"
using namespace kc::Bot::LocaleConst;

namespace kc {

Bot::Locale::Pointer Bot::Locale::Create(Type localeType)
{
    if (localeType == LocaleRu::Type)
        return std::make_unique<LocaleRu>();
    return std::make_unique<LocaleEn>();
}

Bot::Locale::Pointer Bot::Locale::Create(const std::string localeName)
{
    if (localeName == LocaleRu::Name)
        return Create(LocaleRu::Type);
    return Create(LocaleEn::Type);
}

dpp::message Bot::Locale::SuccessMessage(const std::string& string)
{
    dpp::embed embed;
    embed.color = Colors::Success;
    embed.description = fmt::format("{} {}", Emojis::Success, string);
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Locale::ProblemMessage(const std::string& string)
{
    dpp::embed embed;
    embed.color = Colors::Problem;
    embed.description = fmt::format("{} {}", Emojis::Problem, string);
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Locale::ErrorMessage(const std::string& string)
{
    dpp::embed embed;
    embed.color = Colors::Error;
    embed.description = fmt::format("{} {}", Emojis::Error, string);
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Locale::SessionMessage(const SessionStrings& strings, CardinalFunction cardinalFunction, const Session& session)
{
    dpp::message message;
    message.add_embed(dpp::embed().set_color(Colors::Success));
    message.set_flags(dpp::m_ephemeral);
    dpp::embed& embed = message.embeds[0];

    if (!session.playingVideo)
    {
        embed.description = fmt::format("{} **{}**", Emojis::Boring, strings.prettyQuiet);
        embed.add_field(strings.nothingIsPlaying, "");
        return message;
    }

    embed.thumbnail = { session.playingVideo->video.thumbnailUrl() };
    embed.description = fmt::format(
        strings.videoInfo,
        session.playingVideo->video.author(),
        Utility::NiceString(session.playingVideo->video.duration()),
        Utility::NiceString(session.playingVideo->video.uploadDate()),
        Utility::NiceString(session.playingVideo->video.viewCount()),
        cardinalFunction(session.playingVideo->video.viewCount()),
        session.playingRequester->format_username()
    ) + "\n\n";

    size_t itemsShown = 0;
    if (!session.playingPlaylist)
    {
        std::string oldDescription = embed.description;
        embed.description = fmt::format(
            "**[{}]({})**\n",
            session.playingVideo->video.title(),
            session.playingVideo->video.watchUrl()
        );
        if (!session.playingVideo->chapter.name.empty())
        {
            embed.description += fmt::format(
                "[{}. {} [{}]]({})\n",
                session.playingVideo->chapter.number,
                session.playingVideo->chapter.name,
                Utility::NiceString(session.playingVideo->chapter.duration),
                session.playingVideo->video.watchUrl(session.playingVideo->chapter.timestamp)
            );
        }
        embed.description += oldDescription;
    }
    else
    {
        Youtube::Playlist::Iterator iterator = session.playingPlaylist->iterator;
        if (!iterator)
        {
            iterator = session.playingPlaylist->playlist.last();
            if (session.playingVideo->chapter.name.empty())
            {
                embed.description = fmt::format(
                    "**[{}. {}]({})**\n",
                    Utility::NiceString(iterator.index() + 1),
                    iterator->title(),
                    iterator.watchUrl()
                ) + embed.description + fmt::format(
                    ">>> **[{}]({})**\n",
                    session.playingPlaylist->playlist.title(),
                    session.playingPlaylist->playlist.viewUrl()
                ) + fmt::format(
                    strings.lastVideoPlaylistInfo,
                    session.playingPlaylist->playlist.author()
                ) + '\n';
            }
            else
            {
                embed.description = fmt::format(
                    "**[{}. {}]({})**\n"
                    "[{}. {} [{}]]({})\n",
                    Utility::NiceString(iterator.index() + 1),
                    iterator->title(),
                    iterator.watchUrl(),
                    session.playingVideo->chapter.number,
                    session.playingVideo->chapter.name,
                    Utility::NiceString(session.playingVideo->chapter.duration),
                    iterator->watchUrl(session.playingVideo->chapter.timestamp)
                ) + embed.description + fmt::format(
                    ">>> **[{}]({})**\n",
                    session.playingPlaylist->playlist.title(),
                    session.playingPlaylist->playlist.viewUrl()
                ) + fmt::format(
                    strings.lastVideoPlaylistInfo,
                    session.playingPlaylist->playlist.author()
                ) + '\n';
            }
        }
        else
        {
            --iterator;
            std::string oldDescription = embed.description;
            embed.description = fmt::format(
                "**[{}. {}]({})**\n",
                Utility::NiceString(iterator.index() + 1),
                iterator->title(),
                iterator.watchUrl()
            );
            if (!session.playingVideo->chapter.name.empty())
            {
                embed.description += fmt::format(
                    "[{}. {} [{}]]({})\n",
                    session.playingVideo->chapter.number,
                    session.playingVideo->chapter.name,
                    Utility::NiceString(session.playingVideo->chapter.duration),
                    session.playingVideo->video.watchUrl(session.playingVideo->chapter.timestamp)
                );
            }
            embed.description += oldDescription;
            ++iterator;

            size_t videosLeft = session.playingPlaylist->playlist.videoCount() - iterator.index();
            embed.description += fmt::format(
                ">>> **[{}]({})**\n",
                session.playingPlaylist->playlist.title(),
                session.playingPlaylist->playlist.viewUrl()
            ) + fmt::format(
                strings.playlistInfo,
                session.playingPlaylist->playlist.author(),
                Utility::NiceString(videosLeft),
                cardinalFunction(videosLeft)
            ) + '\n';

            for (; iterator; ++iterator, ++itemsShown)
            {
                if (itemsShown == MaxSessionItemsShown)
                {
                    videosLeft = session.playingPlaylist->playlist.videoCount() - iterator.index();
                    embed.add_field("> " + fmt::format(
                        strings.morePlaylistVideos,
                        Utility::NiceString(videosLeft),
                        cardinalFunction(videosLeft)
                    ), "");
                    break;
                }
                
                switch (iterator->type())
                {
                    default:
                    {
                        embed.add_field(fmt::format(
                            "> {}. {}\n",
                            Utility::NiceString(iterator.index() + 1),
                            iterator->title()
                        ), "> " + fmt::format(
                            strings.playlistVideoInfo,
                            iterator->author(),
                            Utility::NiceString(iterator->duration())
                        ));
                        break;
                    }
                    case Youtube::Video::Type::Livestream:
                    {
                        embed.add_field(fmt::format(
                            "> {}. {}\n",
                            Utility::NiceString(iterator.index() + 1),
                            iterator->title()
                        ), "> " + fmt::format(
                            strings.playlistLivestreamInfo,
                            iterator->author()
                        ));
                        break;
                    }
                    case Youtube::Video::Type::Premiere:
                    {
                        embed.add_field(fmt::format(
                            "> {}. {}\n",
                            Utility::NiceString(iterator.index() + 1),
                            iterator->title()
                        ), "> " + fmt::format(
                            strings.playlistPremiereInfo,
                            iterator->author()
                        ));
                        break;
                    }
                }
            }
        }
    }

    if (session.queue.empty())
    {
        embed.add_field(strings.queueIsEmpty, "");
        return message;
    }

    embed.add_field(fmt::format(
        strings.queueInfo,
        Utility::NiceString(session.queue.size()),
        cardinalFunction(session.queue.size())
    ), "");

    if (itemsShown != MaxSessionItemsShown)
    {
        for (size_t index = 0, size = session.queue.size(); index < size; ++index, ++itemsShown)
        {
            if (itemsShown == MaxSessionItemsShown)
            {
                size_t itemsLeft = size - index;
                embed.add_field(fmt::format(
                    strings.moreQueueItems,
                    Utility::NiceString(itemsLeft),
                    cardinalFunction(itemsLeft)
                ), "");
                break;
            }

            const Session::EnqueuedItem& item = session.queue[index];
            switch (item.item.type())
            {
                case Youtube::Item::Type::Video:
                {
                    const Youtube::Video& video = std::get<Youtube::Video>(item.item);
                    embed.add_field(fmt::format(
                        "{}. {}",
                        index + 1,
                        video.title()
                    ), fmt::format(
                        strings.queueVideoInfo,
                        video.author(),
                        Utility::NiceString(video.duration()),
                        item.requester.format_username()
                    ));
                    break;
                }
                case Youtube::Item::Type::Playlist:
                {
                    const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item.item);
                    embed.add_field(fmt::format(
                        "{}. {}",
                        index + 1,
                        playlist.title()
                    ), fmt::format(
                        strings.queuePlaylistInfo,
                        playlist.author(),
                        Utility::NiceString(playlist.videoCount()),
                        cardinalFunction(playlist.videoCount()),
                        item.requester.format_username()
                    ));
                    break;
                }
            }
        }
    }

    return message;
}

dpp::message Bot::Locale::SettingsMessage(const SettingsStrings& strings, const Settings& settings)
{
    dpp::embed embed;
    embed.color = Colors::Success;
    embed.description = fmt::format("{} {}:\n\n", Emojis::Success, strings.hereAreTheSettings);
    embed.description += fmt::format("{}: `{}`\n", strings.language, settings.locale->longName());
    embed.description += fmt::format("{}: `{}`\n", strings.timeoutDuration, Utility::NiceString(pt::time_duration(0, settings.timeoutMinutes, 0)));
    embed.description += fmt::format("{}? `{}`", strings.changeStatus, settings.changeStatus ? strings.yes : strings.no);
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Locale::StatsMessage(const StatsStrings& strings, const Stats& stats)
{
    dpp::embed embed;
    embed.color = Colors::Success;
    embed.description = fmt::format("{} {}\n\n", Emojis::Success, strings.hereAreTheStats);
    embed.description += fmt::format("{}: `{}`\n", strings.interactionsProcessed, Utility::NiceString(stats.interactionsProcessed));
    embed.description += fmt::format("{}: `{}`\n", strings.sessionsCount, Utility::NiceString(stats.sessionsCount));
    embed.description += fmt::format("{}: `{}`", strings.tracksPlayed, Utility::NiceString(stats.tracksPlayed));
    if (stats.timesKicked)
        embed.description += fmt::format("\n{}: `{}`", strings.timesKicked, Utility::NiceString(stats.timesKicked));
    if (stats.timesMoved)
        embed.description += fmt::format("\n{}: `{}`", strings.timesMoved, Utility::NiceString(stats.timesMoved));
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Locale::AmbiguousPlayMessage(const AmbigousPlayStrings& strings, const std::string& videoId, const std::string& playlistId)
{
    dpp::embed embed;
    embed.color = Colors::Question;
    embed.description = fmt::format("{} {}", Emojis::Question, strings.playPlaylistOrVideo);

    dpp::component playVideoButton;
    playVideoButton.type = dpp::cot_button;
    playVideoButton.style = dpp::cos_danger;
    playVideoButton.label = strings.playVideo;
    playVideoButton.set_emoji(Emojis::Play);
    playVideoButton.set_id(Signal(Signal::Type::PlayVideo, videoId));

    dpp::component playPlaylistButton;
    playPlaylistButton.type = dpp::cot_button;
    playPlaylistButton.style = dpp::cos_danger;
    playPlaylistButton.label = strings.playPlaylist;
    playPlaylistButton.set_emoji(Emojis::Playlist);
    playPlaylistButton.set_id(Signal(Signal::Type::PlayPlaylist, playlistId));

    dpp::component buttonComponent;
    buttonComponent.add_component(playVideoButton);
    buttonComponent.add_component(playPlaylistButton);
    return dpp::message().add_embed(embed).add_component(buttonComponent).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Locale::ItemAddedMessage(const ItemAddedStrings& strings, const Youtube::Item& item, CardinalFunction cardinalFunction, const std::optional<dpp::user>& requester)
{
    dpp::embed embed;
    embed.color = Colors::Success;
    if (requester)
    {
        embed.set_author(fmt::format(
            strings.requestedBy,
            requester->format_username()
        ), "", requester->get_avatar_url());
    }

    dpp::component buttonComponent;
    switch (item.type())
    {
        case Youtube::Item::Type::Video:
        {
            const Youtube::Video& video = std::get<Youtube::Video>(item);
            embed.description = fmt::format("{} {}", Emojis::Success, strings.videoAdded);
            embed.add_field(fmt::format("{} [{}]", video.title(), Utility::NiceString(video.duration())), "");
            embed.thumbnail = { video.thumbnailUrl() };

            dpp::component playAgainButton;
            playAgainButton.type = dpp::cot_button;
            playAgainButton.style = dpp::cos_primary;
            playAgainButton.label = strings.playAgain;
            playAgainButton.set_emoji(Emojis::Play);
            playAgainButton.set_id(Signal(Signal::Type::PlayVideo, video.id()));
            buttonComponent.add_component(playAgainButton);

            dpp::component relatedButton;
            relatedButton.type = dpp::cot_button;
            relatedButton.style = dpp::cos_danger;
            relatedButton.label = strings.related;
            relatedButton.set_emoji(Emojis::Search);
            relatedButton.set_id(Signal(Signal::Type::RelatedSearch, video.id()));
            buttonComponent.add_component(relatedButton);

            dpp::component youtubeButton;
            youtubeButton.type = dpp::cot_button;
            youtubeButton.style = dpp::cos_link;
            youtubeButton.label = strings.youtube;
            youtubeButton.url = video.watchUrl();
            buttonComponent.add_component(youtubeButton);
            return dpp::message().add_embed(embed).add_component(buttonComponent);
        }
        case Youtube::Item::Type::Playlist:
        {
            const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
            embed.description = fmt::format("{} {}", Emojis::Success, strings.playlistAdded);
            embed.add_field(fmt::format(strings.playlistInfo, playlist.title(), Utility::NiceString(playlist.videoCount()), cardinalFunction(playlist.videoCount())), "");
            embed.thumbnail = { playlist.thumbnailUrl() };

            dpp::component playAgainButton;
            playAgainButton.type = dpp::cot_button;
            playAgainButton.style = dpp::cos_primary;
            playAgainButton.label = strings.playAgain;
            playAgainButton.set_emoji(Emojis::Play);
            playAgainButton.set_id(Signal(Signal::Type::PlayPlaylist, playlist.id()));
            buttonComponent.add_component(playAgainButton);

            dpp::component youtubeButton;
            youtubeButton.type = dpp::cot_button;
            youtubeButton.style = dpp::cos_link;
            youtubeButton.label = strings.youtube;
            youtubeButton.url = playlist.viewUrl();
            buttonComponent.add_component(youtubeButton);
            return dpp::message().add_embed(embed).add_component(buttonComponent);
        }
    }

    throw std::invalid_argument("kc::Bot::Locale::ItemAddedMessage(): Item is empty");
}

dpp::message Bot::Locale::SearchMessage(const SearchStrings& strings, CardinalFunction cardinalFunction, const Youtube::Results& results)
{
    if (results.empty())
        return ProblemMessage(strings.noResults);

    dpp::component itemSelectMenu;
    itemSelectMenu.type = dpp::cot_selectmenu;
    itemSelectMenu.custom_id = Signal(Signal::Type::SearchSelect, results.query());
    if (results.type() == Youtube::Results::Type::Search)
    {
        itemSelectMenu.placeholder = fmt::format(
            strings.resultCount,
            results.query(),
            results.size(),
            cardinalFunction(results.size())
        );
    }
    else
    {
        itemSelectMenu.placeholder = fmt::format(
            strings.relatedResultCount,
            results.size(),
            cardinalFunction(results.size())
        );
    }

    for (size_t index = 0, size = results.size(); index < size && index <= 20; ++index)
    {
        const Youtube::Item& item = results[index];
        dpp::select_option option;
        switch (item.type())
        {
            case Youtube::Item::Type::Video:
            {
                const Youtube::Video& video = std::get<Youtube::Video>(item);
                option.value = Signal(Signal::Type::PlayVideo, video.id());
                option.label = Utility::Truncate(fmt::format("{}: {}", index + 1, video.title()), 100);
                option.description = Utility::Truncate(fmt::format(
                    strings.videoInfo,
                    video.author(),
                    Utility::NiceString(video.duration())
                ), 100);
                break;
            }
            case Youtube::Item::Type::Playlist:
            {
                const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
                option.value = Signal(Signal::Type::PlayPlaylist, playlist.id());
                option.label = Utility::Truncate(fmt::format("{}: {}", index + 1, playlist.title()), 100);
                option.description = Utility::Truncate(fmt::format(
                    strings.playlistInfo,
                    playlist.author(),
                    Utility::NiceString(playlist.videoCount()),
                    cardinalFunction(playlist.videoCount())
                ), 100);
                break;
            }
        }
        itemSelectMenu.add_select_option(option);
    }

    dpp::component actionRow;
    actionRow.type = dpp::cot_action_row;
    actionRow.add_component(itemSelectMenu);
    return SuccessMessage(strings.whatAreWePlaying).set_flags(0).add_component(actionRow);
}

dpp::command_option_choice Bot::Locale::ItemAutocompleteChoice(const ItemAutocompleteStrings& strings, CardinalFunction cardinalFunction, const Youtube::Item& item)
{
    switch (item.type())
    {
        case Youtube::Item::Type::Video:
        {
            const Youtube::Video& video = std::get<Youtube::Video>(item);
            switch (video.type())
            {
                default:
                {
                    std::string itemDescription = fmt::format(
                        strings.videoDescription,
                        video.author(),
                        Utility::NiceString(video.duration())
                    );
                    return dpp::command_option_choice(fmt::format(
                        "\"{}\"{}",
                        Utility::Truncate(video.title(), 98 - utf8len(itemDescription.c_str())),
                        itemDescription
                    ), video.watchUrl());
                }
                case Youtube::Video::Type::Livestream:
                {
                    std::string itemDescription = fmt::format(
                        strings.livestreamDescription,
                        video.author(),
                        Utility::NiceString(video.duration())
                    );
                    return dpp::command_option_choice(fmt::format(
                        "\"{}\"{}",
                        Utility::Truncate(video.title(), 98 - utf8len(itemDescription.c_str())),
                        itemDescription
                    ), video.watchUrl());
                }
                case Youtube::Video::Type::Premiere:
                {
                    std::string itemDescription = fmt::format(
                        strings.premiereDescription,
                        video.author(),
                        Utility::NiceString(video.duration())
                    );
                    return dpp::command_option_choice(fmt::format(
                        "\"{}\"{}",
                        Utility::Truncate(video.title(), 98 - utf8len(itemDescription.c_str())),
                        itemDescription
                    ), video.watchUrl());
                }
            }
        }
        case Youtube::Item::Type::Playlist:
        {
            const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
            std::string itemDescription = fmt::format(
                strings.playlistDescription,
                playlist.author(),
                Utility::NiceString(playlist.videoCount()),
                cardinalFunction(playlist.videoCount())
            );
            return dpp::command_option_choice(fmt::format(
                "\"{}\"{}",
                Utility::Truncate(playlist.title(), 98 - utf8len(itemDescription.c_str())),
                itemDescription
            ), playlist.viewUrl());
        }
    }

    throw std::invalid_argument("kc::Bot::Locale::ItemAutocompleteChoice(): Item is empty");
}

dpp::message Bot::Locale::EndMessage(const EndStrings& strings, const Settings& settings, EndReason reason, Session session)
{
    dpp::embed embed;
    embed.set_author(fmt::format(
        strings.sessionInfo,
        session.starter.format_username(),
        Utility::NiceString(session.number)
    ), "", session.starter.get_avatar_url());

    switch (reason)
    {
        case EndReason::UserRequested:
        {
            embed.color = Colors::End;
            embed.title = strings.userRequested;
            break;
        }
        case EndReason::Timeout:
        {
            embed.color = Colors::End;
            embed.title = strings.timeout;
            embed.description = strings.timeoutCanBeChanged;
            break;
        }
        case EndReason::EverybodyLeft:
        {
            embed.color = Colors::End;
            embed.title = strings.everybodyLeft;
            break;
        }
        case EndReason::Kicked:
        {
            embed.color = Colors::BadEnd;
            embed.title = strings.kicked;
            if (settings.changeStatus)
                embed.description = strings.voiceStatusNotCleared;
            break;
        }
        case EndReason::Moved:
        {
            embed.color = Colors::BadEnd;
            embed.title = strings.moved;
            if (settings.changeStatus)
                embed.description = strings.voiceStatusNotCleared;
            break;
        }
        default:
        {
            throw std::invalid_argument("kc::Bot::Locale::EndMessage(): Reason is unknown");
        }
    }

    embed.add_field(strings.sessionStats, "");
    embed.fields[0].value += fmt::format(
        "{} `{}`\n",
        strings.lasted,
        Utility::NiceString(session.startTimestamp.is_not_a_date_time() ? pt::time_duration() : pt::second_clock::local_time() - session.startTimestamp)
    );
    embed.fields[0].value += fmt::format(
        "{}: `{}`\n",
        strings.tracksPlayed,
        Utility::NiceString(session.tracksPlayed)
    );
    return dpp::message().add_embed(embed);
}

} // namespace kc
