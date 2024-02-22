#include "bot/embeds.hpp"

namespace kc {

static void CheckRequester(const Bot::Locale::Pointer& locale, dpp::embed& embed, const std::optional<dpp::user>& requester)
{
    if (!requester)
        return;
    embed.set_author(locale->UsedCommand(requester->format_username(), "play"), "", requester->get_avatar_url());
}

dpp::message Bot::Embeds::Joining(const Locale::Pointer& locale, dpp::snowflake channelId)
{
    dpp::embed embed;
    embed.color = Statuses::Success::Color;
    embed.description = Statuses::Success::Prefix + locale->Joining(channelId);
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Embeds::AlreadyJoined(const Locale::Pointer& locale, dpp::snowflake channelId)
{
    dpp::embed embed;
    embed.color = Statuses::Success::Color;
    embed.description = Statuses::Success::Prefix + locale->AlreadyJoined(channelId);
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Embeds::CantJoin(const Locale::Pointer& locale)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + std::string(locale->CantJoin());
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::Left(const Locale::Pointer& locale, dpp::snowflake channelId)
{
    dpp::embed embed;
    embed.color = Statuses::Success::Color;
    embed.description = Statuses::Success::Prefix + locale->Left(channelId);
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Embeds::CantLeave(const Locale::Pointer& locale)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + std::string(locale->CantLeave());
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::AmbigousPlay(const Locale::Pointer& locale, const std::string& videoId, const std::string& playlistId)
{
    dpp::embed embed;
    embed.color = Statuses::Question::Color;
    embed.description = Statuses::Question::Prefix + std::string(locale->AmbigousPlay());

    dpp::component playVideoButton;
    playVideoButton.type = dpp::cot_button;
    playVideoButton.style = dpp::cos_danger;
    playVideoButton.label = locale->PlayVideo();
    playVideoButton.set_emoji(u8"▶️");
    playVideoButton.set_id(Signal(Signal::Type::PlayVideo, videoId));

    dpp::component playPlaylistButton;
    playPlaylistButton.type = dpp::cot_button;
    playPlaylistButton.style = dpp::cos_danger;
    playPlaylistButton.label = locale->PlayPlaylist();
    playPlaylistButton.set_emoji(u8"🔢");
    playPlaylistButton.set_id(Signal(Signal::Type::PlayPlaylist, playlistId));

    dpp::component buttonComponent;
    buttonComponent.add_component(playVideoButton);
    buttonComponent.add_component(playPlaylistButton);
    return dpp::message().add_embed(embed).add_component(buttonComponent).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::ItemAdded(const Locale::Pointer& locale, const Youtube::Item& item, const std::optional<dpp::user>& requester)
{
    dpp::embed embed;
    embed.color = Statuses::Success::Color;
    CheckRequester(locale, embed, requester);
    switch (item.type())
    {
        case Youtube::Item::Type::Video:
        {
            const Youtube::Video& video = std::get<Youtube::Video>(item);
            embed.description = Statuses::Success::Prefix + fmt::format("[{}]({})", locale->ItemAdded(), video.watchUrl());
            embed.url = video.watchUrl();
            embed.add_field(video.title(), "");
            embed.thumbnail = { video.thumbnailUrl() };
            break;
        }
        case Youtube::Item::Type::Playlist:
        {
            const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
            embed.description = Statuses::Success::Prefix + fmt::format("[{}]({})", locale->ItemAdded(), playlist.viewUrl());
            embed.url = playlist.viewUrl();
            embed.add_field(playlist.title(), "");
            embed.thumbnail = { playlist.thumbnailUrl() };
            break;
        }
    }
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Embeds::LivestreamsCantBePlayed(const Locale::Pointer& locale, const Youtube::Video& livestream, const std::optional<dpp::user>& requester)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    CheckRequester(locale, embed, requester);
    embed.description = Statuses::Problem::Prefix + locale->CouldntAdd(livestream.title());
    embed.description += fmt::format("\n```{}```\n", locale->LivestreamsCantBePlayed());
    embed.thumbnail = { livestream.thumbnailUrl() };
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::UpcomingVideosCantBePlayed(const Locale::Pointer& locale, const Youtube::Video& upcomingVideo, const std::optional<dpp::user>& requester)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    CheckRequester(locale, embed, requester);
    embed.description = Statuses::Problem::Prefix + locale->CouldntAdd(upcomingVideo.title());
    embed.description += fmt::format("\n```{}```\n", locale->UpcomingVideosCantBePlayed());
    embed.thumbnail = { upcomingVideo.thumbnailUrl() };
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::CouldntAddYoutubeError(const Locale::Pointer& locale, const Youtube::YoutubeError& error)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + locale->CouldntAdd();
    switch (error.type())
    {
        case Youtube::YoutubeError::Type::LoginRequired:
            embed.description += fmt::format("```{}```", locale->YoutubePrivateVideo());
            break;
        case Youtube::YoutubeError::Type::Unplayable:
            embed.description += fmt::format("```{}```", locale->YoutubeBlockedVideo());
            break;
        case Youtube::YoutubeError::Type::YoutubeError:
            embed.description += fmt::format("```{}```", locale->YoutubeInternalError());
            break;
        case Youtube::YoutubeError::Type::PlaylistError:
            embed.description += fmt::format("```{}```", locale->YoutubePrivatePlaylist());
            break;
        default:
            embed.description += fmt::format("```{}```", locale->YoutubeUnknownError());
            break;
    }
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::CouldntAddLocalError(const Locale::Pointer& locale, const Youtube::LocalError& error)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + locale->CouldntAdd();
    switch (error.type())
    {
        case Youtube::LocalError::Type::PlaylistOfUnplayableVideos:
            embed.description += fmt::format("```{}```", locale->PlaylistOfUnplayableVideos());
            break;
        case Youtube::LocalError::Type::ShortsPlaylist:
            embed.description += fmt::format("```{}```", locale->ShortsPlaylistsCantBePlayed());
            break;
        case Youtube::LocalError::Type::EmptyPlaylist:
            embed.description += fmt::format("```{}```", locale->EmptyPlaylist());
            break;
    }
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::CouldntAddUnknownError(const Locale::Pointer& locale)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + locale->CouldntAdd();
    embed.description += fmt::format("```{}```", locale->SomethingWentWrong());
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::Search(const Locale::Pointer& locale, const Youtube::SearchResult& searchResult)
{
    if (searchResult.items.size() == 0)
    {
        dpp::embed embed;
        embed.color = Statuses::Problem::Color;
        embed.description = Statuses::Problem::Prefix + locale->SearchResult(0);
        return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
    }

    dpp::embed embed;
    embed.color = Statuses::Success::Color;
    embed.description = Statuses::Success::Prefix + std::string(locale->WhatAreWePlaying());

    dpp::component itemSelectMenu;
    itemSelectMenu.type = dpp::cot_selectmenu;
    itemSelectMenu.placeholder = fmt::format("\"{}\": {}", searchResult.query, locale->SearchResult(searchResult.items.size()));
    itemSelectMenu.custom_id = "";
    for (int index = 0, size = searchResult.items.size(); index < size && index <= 20; ++index)
    {
        const Youtube::Item& item = searchResult.items[index];
        switch (item.type())
        {
            case Youtube::Item::Type::Video:
            {
                const Youtube::Video& video = std::get<Youtube::Video>(item);
                dpp::select_option option;
                option.value = Signal(Signal::Type::PlayVideo, video.id());
                option.label = Utility::Truncate(fmt::format("{}: {}", index + 1, video.title()), 100);
                option.description = Utility::Truncate(locale->SearchOptionDescription(video), 100);
                itemSelectMenu.add_select_option(option);
                break;
            }
            case Youtube::Item::Type::Playlist:
            {
                const Youtube::Playlist& playlist = std::get<Youtube::Playlist>(item);
                dpp::select_option option;
                option.value = Signal(Signal::Type::PlayPlaylist, playlist.id());
                option.label = Utility::Truncate(fmt::format("{}: {}", index + 1, playlist.title()), 100);
                option.description = Utility::Truncate(locale->SearchOptionDescription(playlist), 100);
                itemSelectMenu.add_select_option(option);
                break;
            }
        }
    }

    dpp::component actionRow;
    actionRow.type = dpp::cot_action_row;
    actionRow.add_component(itemSelectMenu);
    return dpp::message().add_embed(embed).add_component(actionRow).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::BadSeek(const Locale::Pointer& locale, const std::string& timestampString)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + locale->IncorrectTimestamp(timestampString);
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::NotPlayingToSeek(const Locale::Pointer& locale)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + std::string(locale->NotPlayingToSeek());
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::SeekingTo(const Locale::Pointer& locale, const std::string& timestamp)
{
    dpp::embed embed;
    embed.color = Statuses::Success::Color;
    embed.description = Statuses::Success::Prefix + locale->SeekingTo(timestamp);
    return dpp::message().add_embed(embed);
}

dpp::message Bot::Embeds::UnknownSlashcommand(const Locale::Pointer& locale)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + std::string(locale->UnknownSlashcommand());
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::UnknownButton(const Locale::Pointer& locale)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + std::string(locale->UnknownButton());
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

dpp::message Bot::Embeds::UnknownOption(const Locale::Pointer& locale)
{
    dpp::embed embed;
    embed.color = Statuses::Problem::Color;
    embed.description = Statuses::Problem::Prefix + std::string(locale->UnknownOption());
    return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
}

} // namespace kc
