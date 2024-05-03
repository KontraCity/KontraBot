#include "bot/player.hpp"

namespace kc {

std::vector<Youtube::Video::Chapter>::const_iterator Bot::Player::DeduceChapter(const std::vector<Youtube::Video::Chapter>& chapters, pt::time_duration timestamp)
{
    auto chapterEntry = std::lower_bound(
        chapters.begin(),
        chapters.end(),
        timestamp,
        [](const Youtube::Video::Chapter& chapter, pt::time_duration timestamp) { return chapter.timestamp <= timestamp; }
    );

    if (chapterEntry != chapters.begin())
        chapterEntry -= 1;
    return chapterEntry;
}

void Bot::Player::extractNextVideo(const Info& info)
{
    m_session.playingVideo.reset();
    if (m_session.queue.empty() && !m_session.playingPlaylist)
    {
        m_session.playingRequester.reset();
        return;
    }

    if (m_session.playingPlaylist)
    {
        if (m_session.playingPlaylist->iterator)
        {
            m_session.playingVideo.emplace(Session::PlayingVideo{ *(m_session.playingPlaylist->iterator++) });
            if (!m_session.playingVideo->video.chapters().empty())
                chapterReached(m_session.playingVideo->video.chapters()[0], info);
            return;
        }

        m_session.playingPlaylist.reset();
        if (m_session.queue.empty())
            return;
    }

    Session::EnqueuedItem& nextItem = m_session.queue[0];
    m_session.playingRequester.emplace(std::move(nextItem.requester));
    switch (nextItem.item.type())
    {
        case Youtube::Item::Type::Video:
            m_session.playingVideo.emplace(Session::PlayingVideo{ std::move(std::get<Youtube::Video>(nextItem.item)) });
            if (!m_session.playingVideo->video.chapters().empty())
                chapterReached(m_session.playingVideo->video.chapters()[0], info);
            break;
        case Youtube::Item::Type::Playlist:
            m_session.playingPlaylist.emplace(Session::PlayingPlaylist{ std::move(std::get<Youtube::Playlist>(nextItem.item)), {} });
            m_session.playingPlaylist->iterator = m_session.playingPlaylist->playlist.begin();
            m_session.playingVideo.emplace(Session::PlayingVideo{ *(m_session.playingPlaylist->iterator++) });
            if (!m_session.playingVideo->video.chapters().empty())
                chapterReached(m_session.playingVideo->video.chapters()[0], info);
            break;
    }
    m_session.queue.pop_front();
}

void Bot::Player::incrementPlayedTracks(Info& info)
{
    ++info.stats().tracksPlayed;
    ++m_session.tracksPlayed;
}

void Bot::Player::chapterReached(const Youtube::Video::Chapter& chapter, const Info& info)
{
    if (m_session.playingVideo->chapter.name == chapter.name)
        return;

    m_session.playingVideo->chapter = chapter;
    updateStatus(info);
}

void Bot::Player::checkPlayingVideo()
{
    if (m_session.playingVideo)
        startThread();
    else
        m_timeout.enable();
}

void Bot::Player::setStatus(std::string status)
{
    status = Utility::Truncate(status, 500);
    if (m_session.voiceChannelStatus == status)
        return;

    m_session.voiceChannelStatus = status;
    m_root->channel_set_voice_status(m_session.voiceChannelId, status);
}

void Bot::Player::updateStatus(const Info& info)
{
    if (!info.settings().changeStatus)
    {
        setStatus("");
        return;
    }

    if (!m_session.playingVideo || m_session.playingVideo->video.type() != Youtube::Video::Type::Normal)
    {
        setStatus(info.settings().locale->notPlaying());
        return;
    }

    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;
    std::string prefix = voiceClient->is_paused() ? fmt::format("{} ", info.settings().locale->paused()) : "";

    if (!m_session.playingVideo->chapter.name.empty())
    {
        setStatus(prefix + fmt::format(
            "{}: {} [{}]",
            m_session.playingVideo->chapter.number,
            m_session.playingVideo->chapter.name,
            Utility::NiceString(m_session.playingVideo->chapter.duration)
        ));
        return;
    }

    if (!m_session.playingPlaylist)
    {
        setStatus(prefix + fmt::format(
            "{} [{}]",
            m_session.playingVideo->video.title(),
            Utility::NiceString(m_session.playingVideo->video.duration())
        ));
        return;
    }

    Youtube::Playlist::Iterator iterator = m_session.playingPlaylist->iterator;
    --iterator;
    setStatus(prefix + fmt::format(
        "{}. {} [{}]",
        iterator.index() + 1,
        iterator->title(),
        Utility::NiceString(iterator->duration())
    ));
}

void Bot::Player::threadFunction()
{
    dpp::guild* guild = dpp::find_guild(m_session.guildId);
    std::string videoId;

    {
        std::lock_guard lock(m_mutex);
        if (!m_session.playingVideo)
        {
            m_threadStatus = ThreadStatus::Idle;
            return;
        }

        if (m_session.playingVideo->video.type() != Youtube::Video::Type::Normal)
        {
            dpp::discord_voice_client* voiceClient = getVoiceClient();
            if (!voiceClient)
                return;
            Info info(m_session.guildId);

            switch (m_session.playingVideo->video.type())
            {
                case Youtube::Video::Type::Livestream:
                {
                    m_root->message_create(info.settings().locale->livestreamSkipped(m_session.playingVideo->video).set_channel_id(m_session.textChannelId));
                    voiceClient->insert_marker(Signal(Signal::Type::LivestreamSkipped, m_session.playingVideo->video.id()));
                    m_logger.info("\"{}\": Skipping livestream \"{}\"", guild->name, m_session.playingVideo->video.title());
                    break;
                }
                case Youtube::Video::Type::Premiere:
                {
                    m_root->message_create(info.settings().locale->premiereSkipped(m_session.playingVideo->video).set_channel_id(m_session.textChannelId));
                    voiceClient->insert_marker(Signal(Signal::Type::PremiereSkipped, m_session.playingVideo->video.id()));
                    m_logger.info("\"{}\": Skipping premiere \"{}\"", guild->name, m_session.playingVideo->video.title());
                    break;
                }
            }

            m_threadStatus = ThreadStatus::Idle;
            return;
        }

        m_threadStatus = ThreadStatus::Running;
        videoId = m_session.playingVideo->video.id();
        m_timeout.disable();
    }

    bool errorOccured = false;
    try
    {
        Youtube::Extractor extractor(videoId);
        Youtube::Video::Chapter lastChapter;
        pt::time_duration lastCheckTimestamp;
        while (true)
        {
            Youtube::Extractor::Frame frame = extractor.extractFrame();
            if (frame.empty())
                break;

            {
                std::lock_guard lock(m_mutex);
                if (m_threadStatus == ThreadStatus::Stopped)
                    return;

                dpp::discord_voice_client* voiceClient = getVoiceClient();
                if (!voiceClient)
                    return;

                if (m_session.seekTimestamp != -1)
                {
                    extractor.seekTo(m_session.seekTimestamp);
                    voiceClient->stop_audio();
                    m_session.seekTimestamp = -1;
                }

                if (!m_session.playingVideo->video.chapters().empty())
                {
                    pt::time_duration currentTimestamp(0, 0, 0, frame.timestamp() * 1'000);
                    if (std::abs((currentTimestamp - lastCheckTimestamp).total_seconds()) >= 1)
                    {
                        lastCheckTimestamp = currentTimestamp;
                        auto chapterEntry = DeduceChapter(m_session.playingVideo->video.chapters(), currentTimestamp);
                        if (chapterEntry->timestamp != lastChapter.timestamp)
                        {
                            lastChapter = *chapterEntry;
                            voiceClient->insert_marker(Signal(Signal::Type::ChapterReached, chapterEntry->name));
                        }
                    }
                }
                
                voiceClient->send_audio_raw(reinterpret_cast<uint16_t*>(frame.data()), frame.size());
            }
        }
    }
    catch (const Youtube::YoutubeError& error)
    {
        m_logger.error(
            "\"{}\": Couldn't play \"{}\": YouTube error: {}",
            guild->name,
            m_session.playingVideo->video.id(),
            error.what()
        );
        errorOccured = true;
    }
    catch (const Youtube::LocalError& error)
    {
        m_logger.error(
            "\"{}\": Couldn't play \"{}\": Local error: {}",
            guild->name,
            m_session.playingVideo->video.id(),
            error.what()
        );
        errorOccured = true;
    }
    catch (const std::runtime_error& error)
    {
        m_logger.error(
            "\"{}\": Couldn't play \"{}\": Runtime error: {}",
            guild->name,
            m_session.playingVideo->video.id(),
            error.what()
        );
        errorOccured = true;
    }
    catch (const std::exception& error)
    {
        m_logger.error(
            "\"{}\": Couldn't play \"{}\": Unknown error: {}",
            guild->name,
            m_session.playingVideo->video.id(),
            error.what()
        );
        errorOccured = true;
    }
    catch (...)
    {
        m_logger.error(
            "\"{}\": Couldn't play \"{}\": Unknown error",
            guild->name,
            m_session.playingVideo->video.id()
        );
        errorOccured = true;
    }

    std::lock_guard lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    if (errorOccured)
    {
        Info info(m_session.guildId);
        m_root->message_create(info.settings().locale->playError(m_session.playingVideo->video).set_channel_id(m_session.textChannelId));
        voiceClient->insert_marker(Signal(Signal::Type::PlayError, videoId));
        m_threadStatus = ThreadStatus::Idle;
        return;
    }

    voiceClient->insert_marker(Signal(Signal::Type::Played, videoId));
    m_threadStatus = ThreadStatus::Idle;
}

dpp::discord_voice_client* Bot::Player::getVoiceClient()
{
    dpp::voiceconn* botVoice = m_client->get_voice(m_session.guildId);
    if (!botVoice || !botVoice->voiceclient || !botVoice->is_ready())
        return nullptr;
    return botVoice->voiceclient;
}

void Bot::Player::startThread()
{
    if (m_thread.joinable())
        m_thread.join();
    m_thread = std::thread(&Player::threadFunction, this);
}

void Bot::Player::stopThread(std::unique_lock<std::mutex>& lock)
{
    if (m_threadStatus == ThreadStatus::Running)
        m_threadStatus = ThreadStatus::Stopped;

    if (!m_thread.joinable())
        return;

    lock.unlock();
    m_thread.join();
    lock.lock();
}

void Bot::Player::signalDisconnect(Locale::EndReason reason)
{
    std::thread disconnectThread([this, reason]()
    {
        dpp::guild* guild = dpp::find_guild(m_session.guildId);
        Info info(guild->id);
        m_root->leaveVoice(m_client, *guild, info, reason);
    });
    disconnectThread.detach();
}

Bot::Player::Player(Bot* root, dpp::discord_client* client, const dpp::interaction& interaction, dpp::snowflake voiceChannelId, Info& info)
    : m_logger("player", std::make_shared<spdlog::sinks::stdout_color_sink_mt>())
    , m_root(root)
    , m_timeout([this]() { signalDisconnect(Locale::EndReason::Timeout); }, info.settings().timeoutMinutes * 60)
    , m_client(client)
    , m_session({ 
        interaction.get_guild().id,
        voiceChannelId,
        interaction.channel_id,
        ++info.stats().sessionsCount,
        interaction.get_issuing_user()
    })
{}

Bot::Player::~Player()
{
    if (m_threadStatus == ThreadStatus::Running)
        m_threadStatus = ThreadStatus::Stopped;
    if (m_thread.joinable())
        m_thread.join();
}

void Bot::Player::signalReady(const Info& info)
{
    std::lock_guard lock(m_mutex);
    m_session.startTimestamp = pt::second_clock::local_time();
    if (m_timeout.enabled())
        m_timeout.reset();

    dpp::guild* guild = dpp::find_guild(m_session.guildId);
    if (Bot::CountVoiceMembers(*guild, m_session.voiceChannelId) == 1)
    {
        signalDisconnect(Locale::EndReason::EverybodyLeft);
        return;
    }

    extractNextVideo(info);
    checkPlayingVideo();
    updateStatus(info);
}

void Bot::Player::signalMarker(const Signal& signal, Info& info)
{
    std::lock_guard lock(m_mutex);

    if (signal.type() == Signal::Type::ChapterReached)
    {
        auto chapterEntry = std::find_if(
            m_session.playingVideo->video.chapters().begin(),
            m_session.playingVideo->video.chapters().end(),
            [signal](const Youtube::Video::Chapter& chapter) { return chapter.name == signal.data(); }
        );
        if (chapterEntry != m_session.playingVideo->video.chapters().end())
            chapterReached(*chapterEntry, info);
        return;
    }

    extractNextVideo(info);
    if (signal.type() == Signal::Type::Played)
        incrementPlayedTracks(info);
    checkPlayingVideo();
    updateStatus(info);
}

void Bot::Player::updateTextChannel(dpp::snowflake channelId)
{
    std::lock_guard lock(m_mutex);
    m_session.textChannelId = channelId;
}


void Bot::Player::updateTimeout(const Info& info)
{
    std::lock_guard lock(m_mutex);
    m_timeout.setTimeoutDuration(info.settings().timeoutMinutes * 60);
    if (m_timeout.enabled())
        m_timeout.reset();
}

void Bot::Player::updateVoiceStatus(const Info& info)
{
    std::lock_guard lock(m_mutex);
    updateStatus(info);
}

Bot::Session Bot::Player::session()
{
    std::lock_guard lock(m_mutex);
    return m_session;
}

void Bot::Player::addItem(const Youtube::Item& item, const dpp::user& requester, const Info& info)
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
    {
        m_session.queue.emplace_back(Session::EnqueuedItem{ item, requester });
        return;
    }

    if (!m_session.playingVideo)
    {
        m_session.queue.emplace_back(Session::EnqueuedItem{ item, requester });
        extractNextVideo(info);
        updateStatus(info);
        startThread();
        return;
    }

    m_session.queue.emplace_back(Session::EnqueuedItem{ item, requester });
}

bool Bot::Player::paused()
{
    std::lock_guard lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return false;
    return voiceClient->is_paused();
}

bool Bot::Player::pauseResume(const Info& info)
{
    std::lock_guard lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return false;

    bool isPaused = !voiceClient->is_paused();
    voiceClient->pause_audio(isPaused);
    (isPaused ? m_timeout.enable() : m_timeout.disable());
    updateStatus(info);
    return isPaused;
}

void Bot::Player::seek(uint64_t timestamp, const Info& info)
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    if (!m_session.playingVideo->video.chapters().empty())
    {
        auto chapterEntry = DeduceChapter(m_session.playingVideo->video.chapters(), pt::time_duration(0, 0, timestamp));
        chapterReached(*chapterEntry, info);
    }

    m_session.seekTimestamp = timestamp;
    if (m_threadStatus != ThreadStatus::Running)
        startThread();
}

void Bot::Player::shuffle()
{
    std::lock_guard lock(m_mutex);
    static std::random_device randomDevice;
    static std::default_random_engine randomEngine(randomDevice());
    std::shuffle(m_session.queue.begin(), m_session.queue.end(), randomEngine);
}

void Bot::Player::skipVideo(Info& info)
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    stopThread(lock);
    voiceClient->stop_audio();
    incrementPlayedTracks(info);

    extractNextVideo(info);
    checkPlayingVideo();
    updateStatus(info);
}

void Bot::Player::skipPlaylist(Info& info)
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    stopThread(lock);
    voiceClient->stop_audio();
    incrementPlayedTracks(info);

    m_session.playingPlaylist.reset();
    extractNextVideo(info);
    checkPlayingVideo();
    updateStatus(info);
}

void Bot::Player::clear()
{
    std::unique_lock lock(m_mutex);
    m_session.queue.clear();
}

void Bot::Player::stop(Info& info)
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    stopThread(lock);
    voiceClient->stop_audio();
    incrementPlayedTracks(info);

    m_session.playingVideo.reset();
    m_session.playingPlaylist.reset();
    m_session.queue.clear();
    m_timeout.enable();
    updateStatus(info);
}

void Bot::Player::endSession(Info& info, bool dontClearVoiceStatus)
{
    std::lock_guard lock(m_mutex);
    if (m_session.playingVideo)
        incrementPlayedTracks(info);
    if (!dontClearVoiceStatus)
        setStatus("");
}

void Bot::Player::endSession(Info& info, Locale::EndReason reason)
{
    switch (reason)
    {
        case Locale::EndReason::UserRequested:
        case Locale::EndReason::Timeout:
        {
            endSession(info);
            break;
        }
        case Locale::EndReason::EverybodyLeft:
        {
            /*
            *   Bot is the last user to leave voice channel.
            *   Discord clears voice status automatically when there is no one left.
            */
            endSession(info, true);
            break;
        }
        case Locale::EndReason::Kicked:
        case Locale::EndReason::Moved:
        {
            /*
            *   Bot is already not in voice channel.
            *   Voice status can only be modified when bot is in the voice.
            */
            endSession(info, true);
            break;
        }
    }

    m_root->message_create(info.settings().locale->sessionEnd(info.settings(), reason, m_session).set_channel_id(m_session.textChannelId));
}

} // namespace kc
