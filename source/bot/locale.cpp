#include "bot/locale/locale.hpp"
using namespace kb::Bot::LocaleConst;

// STL modules
#include <stdexcept>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locales.hpp"
#include "bot/commands.hpp"
#include "bot/signal.hpp"
#include "core/utility.hpp"

namespace kb {

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

const char* Bot::Locale::EndReasonToString(EndReason reason)
{
    switch (reason)
    {
        case EndReason::UserRequested:
            return "requested by user";
        case EndReason::Timeout:
            return "timeouted";
        case EndReason::EverybodyLeft:
            return "everybody left";
        case EndReason::Kicked:
            return "kicked";
        case EndReason::Moved:
            return "moved";
        default:
            return "unknown";
    }
}

dpp::message Bot::Locale::GenericMessage(uint32_t color, const char* emoji, const std::string& string, bool ephemeral)
{
    dpp::embed embed;
    embed.color = color;
    embed.description = fmt::format("{} **{}**", emoji, string);
    return dpp::message().add_embed(embed).set_flags(ephemeral ? dpp::m_ephemeral : 0);
}

dpp::message Bot::Locale::SuccessMessage(const std::string& string)
{
    return GenericMessage(Colors::Success, Emojis::Success, string, false);
}

dpp::message Bot::Locale::QuestionMessage(const std::string& string)
{
    return GenericMessage(Colors::Question, Emojis::Question, string, true);
}

dpp::message Bot::Locale::ProblemMessage(const std::string& string)
{
    return GenericMessage(Colors::Problem, Emojis::Problem, string, true);
}

dpp::message Bot::Locale::ErrorMessage(const std::string& string)
{
    return GenericMessage(Colors::Error, Emojis::Error, string, true);
}

dpp::message Bot::Locale::MentionReplyMessage(const MentionReplyStrings& strings)
{
    dpp::embed embed;
    embed.color = Colors::Success;
    embed.description = fmt::format(strings.ifYouNeedAnyHelp, Utility::NiceString(Commands::Instance->help()));
    embed.add_field("", fmt::format(strings.differentLanguagesHint, Utility::NiceString(Commands::Instance->set(), Commands::Instance->set().options[0])));
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Locale::HelpMessage(const HelpStrings& strings)
{
    dpp::embed embed;
    embed.color = Colors::Success;
    embed.description = fmt::format("{} **{}**:", Emojis::Success, strings.hereAreAllOfMyCommands);
    embed.add_field("", fmt::format(strings.differentLanguagesHint, Utility::NiceString(Commands::Instance->set(), Commands::Instance->set().options[0])));
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->help()), strings.helpDescription), "");
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->session()), strings.sessionDescription), strings.sessionFaq);
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->settings()), strings.settingsDescription), "");
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->stats()), strings.statsDescription), "");
    embed.add_field(fmt::format(
        "{}, {}, {}\n{}",
        Utility::NiceString(Commands::Instance->set(), Commands::Instance->set().options[0]),
        Utility::NiceString(Commands::Instance->set(), Commands::Instance->set().options[1]),
        Utility::NiceString(Commands::Instance->set(), Commands::Instance->set().options[2]),
        strings.setDescription
    ), strings.setFaq);
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->join()), strings.joinDescription), "");
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->leave()), strings.leaveDescription), "");
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->play()), strings.playDescription), strings.playFaq);
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->pause()), strings.pauseDescription), "");
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->seek()), strings.seekDescription), strings.seekFaq);
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->shuffle()), strings.shuffleDescription), "");
    embed.add_field(fmt::format(
        "{}, {}\n{}",
        Utility::NiceString(Commands::Instance->skip(), Commands::Instance->skip().options[0]),
        Utility::NiceString(Commands::Instance->skip(), Commands::Instance->skip().options[1]),
        strings.skipDescription
    ), "");
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->clear()), strings.clearDescription), "");
    embed.add_field(fmt::format("{}\n{}", Utility::NiceString(Commands::Instance->stop()), strings.stopDescription), "");
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Locale::SessionMessage(const SessionStrings& strings, CardinalFunction cardinalFunction, const Session& session)
{
    dpp::message message;
    message.add_embed(dpp::embed().set_color(Colors::Success));
    message.set_flags(dpp::m_ephemeral);
    dpp::embed& embed = message.embeds[0];
    embed.set_footer(fmt::format(
        strings.infoFooter,
        Utility::NiceString(pt::second_clock::local_time() - session.startTimestamp),
        session.tracksPlayed
    ), "");

    if (!session.playingVideo)
    {
        embed.add_field(fmt::format("{} **{}**", Emojis::Boring, strings.prettyQuiet), strings.nothingIsPlaying);
        return message;
    }

    embed.thumbnail = { session.playingVideo->video.thumbnailUrl() };
    embed.description = fmt::format(
        strings.videoInfo,
        session.playingVideo->video.author(),
        Utility::NiceString(session.playingVideo->video.duration()),
        Utility::NiceString(session.playingVideo->video.uploadDate()),
        Utility::NiceString(session.playingVideo->video.viewCount()),
        cardinalFunction(session.playingVideo->video.viewCount())
    );

    size_t itemsShown = 0;
    if (!session.playingPlaylist)
    {
        std::string oldDescription = fmt::format(
            "{}\n{}\n\n",
            embed.description,
            fmt::format(strings.requestedBy, session.playingRequester->format_username())
        );
        embed.description = fmt::format(
            "**[{}]({})**\n",
            session.playingVideo->video.title(),
            session.playingVideo->video.watchUrl()
        );
        if (!session.playingVideo->chapter.name.empty())
        {
            embed.description += fmt::format(
                "[{} #{}: {} [{}]]({})\n",
                strings.chapter,
                Utility::NiceString(session.playingVideo->chapter.number),
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
                    "**[{} #{}: {}]({})**\n",
                    strings.video,
                    Utility::NiceString(iterator.index() + 1),
                    iterator->title(),
                    iterator.watchUrl()
                ) + embed.description + fmt::format(
                    "\n\n>>> **[{}]({})**\n",
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
                    "**[{} #{}: {}]({})**\n"
                    "[{} #{}: {} [{}]]({})\n",
                    strings.video,
                    Utility::NiceString(iterator.index() + 1),
                    iterator->title(),
                    iterator.watchUrl(),
                    strings.chapter,
                    Utility::NiceString(session.playingVideo->chapter.number),
                    session.playingVideo->chapter.name,
                    Utility::NiceString(session.playingVideo->chapter.duration),
                    iterator->watchUrl(session.playingVideo->chapter.timestamp)
                ) + embed.description + "\n\n" + fmt::format(
                    ">>> **[{}]({})**\n",
                    session.playingPlaylist->playlist.title(),
                    session.playingPlaylist->playlist.viewUrl()
                ) + fmt::format(
                    strings.lastVideoPlaylistInfo,
                    session.playingPlaylist->playlist.author()
                ) + fmt::format(
                    "\n{}\n",
                    fmt::format(strings.requestedBy, session.playingRequester->format_username())
                );
            }
        }
        else
        {
            --iterator;
            std::string oldDescription = embed.description;
            embed.description = fmt::format(
                "**[{} #{}: {}]({})**\n",
                strings.video,
                Utility::NiceString(iterator.index() + 1),
                iterator->title(),
                iterator.watchUrl()
            );
            if (!session.playingVideo->chapter.name.empty())
            {
                embed.description += fmt::format(
                    "[{} #{}: {} [{}]]({})\n",
                    strings.chapter,
                    Utility::NiceString(session.playingVideo->chapter.number),
                    session.playingVideo->chapter.name,
                    Utility::NiceString(session.playingVideo->chapter.duration),
                    session.playingVideo->video.watchUrl(session.playingVideo->chapter.timestamp)
                );
            }
            embed.description += oldDescription;
            ++iterator;

            size_t videosLeft = session.playingPlaylist->playlist.videoCount() - iterator.index();
            embed.description += fmt::format(
                "\n\n>>> **[{}]({})**\n",
                session.playingPlaylist->playlist.title(),
                session.playingPlaylist->playlist.viewUrl()
            ) + fmt::format(
                strings.playlistInfo,
                session.playingPlaylist->playlist.author(),
                Utility::NiceString(videosLeft),
                cardinalFunction(videosLeft)
            ) + fmt::format(
                "\n{}\n",
                fmt::format(strings.requestedBy, session.playingRequester->format_username())
            );

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
                        Utility::NiceString(video.duration())
                    ) + '\n' + fmt::format(
                        strings.requestedBy,
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
                        cardinalFunction(playlist.videoCount())
                    ) + '\n' + fmt::format(
                        strings.requestedBy,
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
    embed.add_field(fmt::format("{} {}:", Emojis::Success, strings.hereAreTheSettings), "");
    embed.fields[0].value = fmt::format("{}: **`{}`**\n", strings.language, settings.locale->longName());
    embed.fields[0].value += fmt::format("{}: **`{}`**\n", strings.timeoutDuration, Utility::NiceString(pt::time_duration(0, settings.timeoutMinutes, 0)));
    embed.fields[0].value += fmt::format("{}: **`{}`**", strings.changeStatus, settings.changeStatus ? strings.yes : strings.no);
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Locale::StatsMessage(const StatsStrings& strings, const Stats& stats)
{
    dpp::embed embed;
    embed.color = Colors::Success;
    embed.add_field(fmt::format("{} {}:", Emojis::Success, strings.hereAreTheStats), "");
    embed.fields[0].value = fmt::format("{}: **`{}`**\n", strings.interactionsProcessed, Utility::NiceString(stats.interactionsProcessed));
    embed.fields[0].value += fmt::format("{}: **`{}`**\n", strings.sessionsConducted, Utility::NiceString(stats.sessionsConducted));
    embed.fields[0].value += fmt::format("{}: **`{}`**", strings.tracksPlayed, Utility::NiceString(stats.tracksPlayed));
    if (stats.timesKicked)
        embed.fields[0].value += fmt::format("\n{}: **`{}`**", strings.timesKicked, Utility::NiceString(stats.timesKicked));
    if (stats.timesMoved)
        embed.fields[0].value += fmt::format("\n{}: **`{}`**", strings.timesMoved, Utility::NiceString(stats.timesMoved));
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Locale::AmbiguousPlayMessage(const AmbigousPlayStrings& strings, const std::string& videoId, const std::string& playlistId)
{
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
    return QuestionMessage(strings.playPlaylistOrVideo).add_component(buttonComponent);
}

dpp::message Bot::Locale::ItemAddedMessage(const ItemAddedStrings& strings, const Youtube::Item& item, CardinalFunction cardinalFunction, const std::optional<dpp::user>& requester)
{
    switch (item.type())
    {
        case Youtube::Item::Type::Video:
        {
            dpp::message message = SuccessMessage(strings.videoAdded);
            dpp::embed& embed = message.embeds[0];
            if (requester)
            {
                embed.set_author(fmt::format(
                    strings.requestedBy,
                    requester->format_username()
                ), "", requester->get_avatar_url());
            }

            const Youtube::Video& video = std::get<Youtube::Video>(item);
            embed.add_field(fmt::format("{} [{}]", video.title(), Utility::NiceString(video.duration())), "");
            embed.thumbnail = { video.thumbnailUrl() };

            dpp::component playAgainButton;
            playAgainButton.type = dpp::cot_button;
            playAgainButton.style = dpp::cos_primary;
            playAgainButton.label = strings.playAgain;
            playAgainButton.set_emoji(Emojis::Play);
            playAgainButton.set_id(Signal(Signal::Type::PlayVideo, video.id()));

            dpp::component relatedButton;
            relatedButton.type = dpp::cot_button;
            relatedButton.style = dpp::cos_danger;
            relatedButton.label = strings.related;
            relatedButton.set_emoji(Emojis::Search);
            relatedButton.set_id(Signal(Signal::Type::RelatedSearch, video.id()));

            dpp::component youtubeButton;
            youtubeButton.type = dpp::cot_button;
            youtubeButton.style = dpp::cos_link;
            youtubeButton.label = strings.youtube;
            youtubeButton.url = video.watchUrl();

            dpp::component buttonComponent;
            buttonComponent.add_component(playAgainButton);
            buttonComponent.add_component(relatedButton);
            buttonComponent.add_component(youtubeButton);
            return message.add_component(buttonComponent);
        }
        case Youtube::Item::Type::Playlist:
        {
            dpp::message message = SuccessMessage(strings.playlistAdded);
            dpp::embed& embed = message.embeds[0];
            if (requester)
            {
                embed.set_author(fmt::format(
                    strings.requestedBy,
                    requester->format_username()
                ), "", requester->get_avatar_url());
            }

            const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
            embed.add_field(fmt::format(strings.playlistInfo, playlist.title(), Utility::NiceString(playlist.videoCount()), cardinalFunction(playlist.videoCount())), "");
            embed.thumbnail = { playlist.thumbnailUrl() };

            dpp::component playAgainButton;
            playAgainButton.type = dpp::cot_button;
            playAgainButton.style = dpp::cos_primary;
            playAgainButton.label = strings.playAgain;
            playAgainButton.set_emoji(Emojis::Play);
            playAgainButton.set_id(Signal(Signal::Type::PlayPlaylist, playlist.id()));

            dpp::component youtubeButton;
            youtubeButton.type = dpp::cot_button;
            youtubeButton.style = dpp::cos_link;
            youtubeButton.label = strings.youtube;
            youtubeButton.url = playlist.viewUrl();

            dpp::component buttonComponent;
            buttonComponent.add_component(playAgainButton);
            buttonComponent.add_component(youtubeButton);
            return message.add_component(buttonComponent);
        }
        default:
        {
            throw std::runtime_error(fmt::format(
                "kb::Bot::Locale::ItemAddedMessage(): Item type is unknown [item: {}]",
                static_cast<int>(item.type())
            ));
        }
    }
}

dpp::message Bot::Locale::SearchMessage(const SearchStrings& strings, CardinalFunction cardinalFunction, const Youtube::Results& results)
{
    if (results.empty())
        return ProblemMessage(strings.noResults);

    dpp::component itemSelectMenu;
    itemSelectMenu.type = dpp::cot_selectmenu;
    itemSelectMenu.custom_id = Utility::Truncate(Signal(Signal::Type::SearchSelect, results.query()), 100);
    if (results.type() == Youtube::Results::Type::Search)
    {
        itemSelectMenu.placeholder = Utility::Truncate(fmt::format(
            strings.resultCount,
            results.query(),
            results.size(),
            cardinalFunction(results.size())
        ), 150);
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
            default:
                continue;
        }
        itemSelectMenu.add_select_option(option);
    }

    dpp::component actionRow;
    actionRow.type = dpp::cot_action_row;
    actionRow.add_component(itemSelectMenu);
    return SuccessMessage(strings.whatAreWePlaying).set_flags(0).add_component(actionRow);
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
            embed.description = fmt::format(
                strings.timeoutCanBeChanged,
                Utility::NiceString(Commands::Instance->set(),Commands::Instance->set().options[1])
            );
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
            throw std::runtime_error(fmt::format(
                "kb::Bot::Locale::EndMessage(): Reason is unknown [reason: {}]",
                static_cast<int>(reason)
            ));
        }
    }

    embed.add_field(strings.sessionStats, "");
    embed.fields[0].value += fmt::format(
        "{} **`{}`**\n",
        strings.lasted,
        Utility::NiceString(session.startTimestamp.is_not_a_date_time() ? pt::time_duration() : pt::second_clock::local_time() - session.startTimestamp)
    );
    embed.fields[0].value += fmt::format(
        "{}: **`{}`**\n",
        strings.tracksPlayed,
        Utility::NiceString(session.tracksPlayed)
    );
    return dpp::message().add_embed(embed);
}

} // namespace kb
