#include "bot/player.hpp"

namespace kc {

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
            m_session.playingVideo.emplace(*(m_session.playingPlaylist->iterator++));
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
            m_session.playingVideo.emplace(std::move(std::get<Youtube::Video>(nextItem.item)));
            break;
        case Youtube::Item::Type::Playlist:
            m_session.playingPlaylist.emplace(Session::PlayingPlaylist{ std::move(std::get<Youtube::Playlist>(nextItem.item)), {} });
            m_session.playingPlaylist->iterator = m_session.playingPlaylist->playlist.begin();
            m_session.playingVideo.emplace(*(m_session.playingPlaylist->iterator++));
            break;
    }
    m_session.queue.pop_front();
}

void Bot::Player::incrementPlayedTracks()
{
    Info info(m_session.guildId);
    ++info.stats().tracksPlayed;
    ++m_session.tracksPlayed;
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

        m_threadStatus = ThreadStatus::Running;
        videoId = m_session.playingVideo->id();
        m_timeout.disable();
    }

    bool successfullyPlayed = true;
    try
    {
        Youtube::Extractor extractor(videoId);
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

                voiceClient->send_audio_opus(frame.data(), frame.size());
            }
        }
    }
    catch (...)
    {
        successfullyPlayed = false;
    }

    std::lock_guard lock(m_mutex);
    dpp::discord_voice_client* voiceClient = getVoiceClient();
    if (!voiceClient)
        return;

    voiceClient->insert_marker(Signal((successfullyPlayed ? Signal::Type::Played : Signal::Type::PlayError), videoId));
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

Bot::Player::Player(Bot* root, dpp::discord_client* client, const dpp::interaction& interaction, Info& info)
    : m_root(root)
    , m_timeout([this]() { timeoutHandler(); }, info.settings().timeoutMinutes * 60)
    , m_client(client)
    , m_session({ 
        interaction.get_guild().id,
        interaction.channel_id,
        ++info.stats().sessionsCount,
        interaction.get_issuing_user(),
        pt::second_clock::local_time()
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

void Bot::Player::signalMarker()
{
    std::lock_guard lock(m_mutex);

    extractNextVideo();
    incrementPlayedTracks();
    checkPlayingVideo();
}

Bot::Player::Session Bot::Player::session()
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
}

} // namespace kc
