#include "bot/player/player.hpp"

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

void Bot::Player::extractNextVideo()
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
                chapterReached(m_session.playingVideo->chapter = m_session.playingVideo->video.chapters()[0]);
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
                chapterReached(m_session.playingVideo->chapter = m_session.playingVideo->video.chapters()[0]);
            break;
        case Youtube::Item::Type::Playlist:
            m_session.playingPlaylist.emplace(Session::PlayingPlaylist{ std::move(std::get<Youtube::Playlist>(nextItem.item)), {} });
            m_session.playingPlaylist->iterator = m_session.playingPlaylist->playlist.begin();
            m_session.playingVideo.emplace(Session::PlayingVideo{ *(m_session.playingPlaylist->iterator++) });
            if (!m_session.playingVideo->video.chapters().empty())
                chapterReached(m_session.playingVideo->chapter = m_session.playingVideo->video.chapters()[0]);
            break;
    }
    m_session.queue.pop_front();
}

void Bot::Player::incrementPlayedTracks()
{
    //Info info(m_session.guildId);
    //++info.stats().tracksPlayed;
    ++m_session.tracksPlayed;
}

void Bot::Player::chapterReached(const Youtube::Video::Chapter& chapter)
{
    if (m_session.playingVideo->chapter.timestamp == chapter.timestamp)
        return;

    m_session.playingVideo->chapter = chapter;
    m_logger.critical(chapter.name);
}

void Bot::Player::checkPlayingVideo()
{
    if (m_session.playingVideo)
        startThread();
    else
        m_timeout.enable();
}

void Bot::Player::threadFunction()
{
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
            dpp::guild* guild = dpp::find_guild(m_session.guildId);
            dpp::discord_voice_client* voiceClient = getVoiceClient();
            if (!voiceClient)
                return;
            Info info(m_session.guildId);

            switch (m_session.playingVideo->video.type())
            {
                case Youtube::Video::Type::Livestream:
                {
                    m_root->message_create(info.settings().locale->livestreamSkipped(m_session.playingVideo->video.title()).set_channel_id(m_session.textChannelId));
                    voiceClient->insert_marker(Signal(Signal::Type::LivestreamSkipped, videoId));
                    m_logger.info("\"{}\": Skipping livestream \"{}\"", guild->name, m_session.playingVideo->video.title());
                    break;
                }
                case Youtube::Video::Type::Premiere:
                {
                    m_root->message_create(info.settings().locale->premiereSkipped(m_session.playingVideo->video.title()).set_channel_id(m_session.textChannelId));
                    voiceClient->insert_marker(Signal(Signal::Type::PremiereSkipped, videoId));
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

    bool error = false;
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
                
                voiceClient->send_audio_opus(frame.data(), frame.size());
            }
        }
    }
    catch (...)
    {
        error = true;
    }

    std::lock_guard lock(m_mutex);
    dpp::guild* guild = dpp::find_guild(m_session.guildId);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    if (error)
    {
        Info info(m_session.guildId);
        m_root->message_create(info.settings().locale->playError(m_session.playingVideo->video.title()).set_channel_id(m_session.textChannelId));
        voiceClient->insert_marker(Signal(Signal::Type::PlayError, videoId));
        m_logger.error("\"{}\": Couldn't play \"{}\"", guild->name, m_session.playingVideo->video.title());
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

void Bot::Player::timeoutHandler()
{
    std::thread disconnectThread([this]() { m_root->leaveVoice(m_client, *dpp::find_guild(m_session.guildId), Locale::EndReason::Timeout); });
    disconnectThread.detach();
}

Bot::Player::Player(Bot* root, dpp::discord_client* client, const dpp::interaction& interaction, dpp::snowflake voiceChannelId, Info& info)
    : m_logger("player", std::make_shared<spdlog::sinks::stdout_color_sink_mt>())
    , m_root(root)
    , m_timeout([this]() { timeoutHandler(); }, info.settings().timeoutMinutes * 60)
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

void Bot::Player::signalReady()
{
    std::lock_guard lock(m_mutex);

    getVoiceClient()->set_send_audio_type(dpp::discord_voice_client::satype_overlap_audio);
    m_session.startTimestamp = pt::second_clock::local_time();
    if (m_timeout.enabled())
        m_timeout.reset();

    extractNextVideo();
    checkPlayingVideo();
}

void Bot::Player::signalMarker(const Signal& signal)
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
            chapterReached(*chapterEntry);
        return;
    }

    extractNextVideo();
    if (signal.type() == Signal::Type::Played)
        incrementPlayedTracks();
    checkPlayingVideo();
}

void Bot::Player::updateTextChannel(dpp::snowflake channelId)
{
    std::lock_guard lock(m_mutex);
    m_session.textChannelId = channelId;
}

Bot::Session Bot::Player::session()
{
    std::lock_guard lock(m_mutex);
    return m_session;
}

void Bot::Player::addItem(const Youtube::Item& item, const dpp::user& requester)
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
        extractNextVideo();
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

bool Bot::Player::pauseResume()
{
    std::lock_guard lock(m_mutex);

    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return true;

    bool isPaused = !voiceClient->is_paused();
    voiceClient->pause_audio(isPaused);
    (isPaused ? m_timeout.enable() : m_timeout.disable());
    return isPaused;
}

void Bot::Player::seek(uint64_t timestamp)
{
    std::unique_lock lock(m_mutex);

    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    if (!m_session.playingVideo->video.chapters().empty())
    {
        auto chapterEntry = DeduceChapter(m_session.playingVideo->video.chapters(), pt::time_duration(0, 0, timestamp));
        chapterReached(*chapterEntry);
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

void Bot::Player::skipVideo()
{
    std::unique_lock lock(m_mutex);

    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    stopThread(lock);
    voiceClient->stop_audio();
    incrementPlayedTracks();

    extractNextVideo();
    checkPlayingVideo();
}

void Bot::Player::skipPlaylist()
{
    std::unique_lock lock(m_mutex);

    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    stopThread(lock);
    voiceClient->stop_audio();
    incrementPlayedTracks();

    m_session.playingPlaylist.reset();
    extractNextVideo();
    checkPlayingVideo();
}

void Bot::Player::clear()
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    m_session.queue.clear();
}

void Bot::Player::stop()
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    stopThread(lock);
    voiceClient->stop_audio();
    incrementPlayedTracks();

    m_session.playingVideo.reset();
    m_session.playingPlaylist.reset();
    m_session.queue.clear();
    m_timeout.enable();
}

void Bot::Player::endSession(Locale::EndReason reason)
{
    if (m_session.playingVideo)
        incrementPlayedTracks();

    Info info(m_session.guildId);
    m_root->message_create(info.settings().locale->sessionEnd(reason, m_session).set_channel_id(m_session.textChannelId));
}

} // namespace kc
