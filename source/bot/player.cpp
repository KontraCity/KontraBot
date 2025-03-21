#include "bot/player.hpp"

// STL modules
#include <random>
#include <algorithm>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "bot/locale/locale_en.hpp"
#include "bot/bot.hpp"
#include "core/downloader.hpp"
#include "core/utility.hpp"

namespace kb {

/* Temporarily unsupported!
    /// @brief Deduce frame's chapter from frame timestamp
    /// @param chapters Video chapters
    /// @param timestamp Frame's timestamp
    /// @return Frame's chapter
    static std::vector<ytcpp::Video::Chapter>::const_iterator DeduceChapter(const std::vector<ytcpp::Video::Chapter>& chapters, pt::time_duration timestamp)
    {
        auto chapterEntry = std::lower_bound(
            chapters.begin(),
            chapters.end(),
            timestamp,
            [](const ytcpp::Video::Chapter& chapter, pt::time_duration timestamp) { return chapter.timestamp <= timestamp; }
        );

        if (chapterEntry != chapters.begin())
            chapterEntry -= 1;
        return chapterEntry;
    }
*/

Bot::Player::Player(Bot* root, dpp::discord_client* client, const dpp::interaction& interaction, dpp::snowflake voiceChannelId, Info& info)
    : m_logger(Utility::CreateLogger(fmt::format("player \"{}\"", interaction.get_guild().name)))
    , m_root(root)
    , m_timeout([this]() { signalDisconnect(Locale::EndReason::Timeout); }, info.settings().timeoutMinutes * 60)
    , m_client(client)
    , m_session({ 
        interaction.get_guild().id,
        voiceChannelId,
        interaction.channel_id,
        info.stats().sessionsConducted += 1,
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
            /* Temporarily unsupported!
            if (!m_session.playingVideo->video.chapters().empty())
                chapterReached(m_session.playingVideo->video.chapters()[0], info);
            */
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
        case ytcpp::Item::Type::Video:
            m_session.playingVideo.emplace(Session::PlayingVideo{ std::move(std::get<ytcpp::Video>(nextItem.item)) });
            /* Temporarily unsupported!
            if (!m_session.playingVideo->video.chapters().empty())
                chapterReached(m_session.playingVideo->video.chapters()[0], info);
            */
            break;
        case ytcpp::Item::Type::Playlist:
            m_session.playingPlaylist.emplace(Session::PlayingPlaylist{ std::move(std::get<ytcpp::Playlist>(nextItem.item)), {} });
            m_session.playingPlaylist->iterator = m_session.playingPlaylist->playlist.begin();
            m_session.playingVideo.emplace(Session::PlayingVideo{ *(m_session.playingPlaylist->iterator++) });
            /* Temporarily unsupported!
            if (!m_session.playingVideo->video.chapters().empty())
                chapterReached(m_session.playingVideo->video.chapters()[0], info);
            */
            break;
    }
    m_session.queue.pop_front();
}

void Bot::Player::incrementPlayedTracks(Info& info)
{
    ++info.stats().tracksPlayed;
    ++m_session.tracksPlayed;
}

/* Temporarily unsupported!
void Bot::Player::chapterReached(const ytcpp::Video::Chapter& chapter, const Info& info)
{
    if (m_session.playingVideo->chapter.name == chapter.name)
        return;

    m_session.playingVideo->chapter = chapter;
    updateStatus(info);
}
*/

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

    if (!m_session.playingVideo || m_session.playingVideo->video.isLivestream() || m_session.playingVideo->video.isUpcoming())
    {
        setStatus(info.settings().locale->notPlaying());
        return;
    }

    dpp::discord_voice_client* client = getVoiceClient();
    if (!client)
        return;
    std::string prefix = client->is_paused() ? fmt::format("{} ", info.settings().locale->paused()) : "";

/* Temporarily unsupported!
    if (!m_session.playingVideo->chapter.name.empty())
    {
        setStatus(prefix + fmt::format(
            "{} #{}: {} [{}]",
            info.settings().locale->chapter(),
            Utility::NiceString(m_session.playingVideo->chapter.number),
            m_session.playingVideo->chapter.name,
            Utility::NiceString(m_session.playingVideo->chapter.duration)
        ));
        return;
    }
*/

    if (!m_session.playingPlaylist)
    {
        setStatus(prefix + fmt::format(
            "{} [{}]",
            m_session.playingVideo->video.title(),
            Utility::NiceString(m_session.playingVideo->video.duration())
        ));
        return;
    }

    ytcpp::Playlist::Iterator iterator = m_session.playingPlaylist->iterator;
    --iterator;
    setStatus(prefix + fmt::format(
        "{} #{}: {} [{}]",
        info.settings().locale->video(),
        Utility::NiceString(iterator.index() + 1),
        iterator->title(),
        Utility::NiceString(iterator->duration())
    ));
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

        if (m_session.playingVideo->video.isLivestream() || m_session.playingVideo->video.isUpcoming())
        {
            dpp::discord_voice_client* client = getVoiceClient();
            if (!client)
                return;

            if (m_session.playingVideo->video.isLivestream()) {
                client->insert_marker(Signal(Signal::Type::LivestreamSkipped, m_session.playingVideo->video.id()));
                m_logger.info("Skipping livestream \"{}\"", m_session.playingVideo->video.title());
            }
            else if (m_session.playingVideo->video.isUpcoming()) {
                client->insert_marker(Signal(Signal::Type::PremiereSkipped, m_session.playingVideo->video.id()));
                m_logger.info("Skipping premiere \"{}\"", m_session.playingVideo->video.title());
            }
            return;
        }

        m_threadStatus = ThreadStatus::Running;
        videoId = m_session.playingVideo->video.id();
        m_timeout.disable();
    }

    bool errorOccured = false;
    try
    {
        Downloader downloader(videoId); 
/* Temporarily unsupported!
        ytcpp::Video::Chapter lastChapter;
*/
        pt::time_duration lastCheckTimestamp;
        while (true)
        {
            Downloader::Frame frame = downloader.extractFrame();
            if (frame.empty())
                break;

            {
                std::lock_guard lock(m_mutex);
                if (m_threadStatus == ThreadStatus::Stopped)
                    return;

                if (m_session.seekTimestamp != -1)
                {
                    dpp::discord_voice_client* client = getVoiceClient();
                    if (!client)
                    {
                        m_threadStatus = ThreadStatus::Idle;
                        return;
                    }
                    client->stop_audio();

                    downloader.seekTo(m_session.seekTimestamp);
                    m_session.seekTimestamp = -1;
                }

/* Temporarily unsupported!
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

                            dpp::discord_voice_client* client = getVoiceClient();
                            if (!client)
                            {
                                m_threadStatus = ThreadStatus::Idle;
                                return;
                            }
                            client->insert_marker(Signal(Signal::Type::ChapterReached, chapterEntry->name));
                        }
                    }
                }
*/

                dpp::discord_voice_client* client = getVoiceClient();
                if (!client)
                {
                    m_threadStatus = ThreadStatus::Idle;
                    return;
                }
                client->send_audio_raw(reinterpret_cast<uint16_t*>(frame.data()), frame.size());
            }
        }
    }
    catch (const ytcpp::YtError& error)
    {
        m_logger.error(
            "Couldn't play \"{}\": YouTube error: {}",
            m_session.playingVideo->video.id(),
            error.what()
        );
        errorOccured = true;
    }
    catch (const ytcpp::Error& error)
    {
        m_logger.error(
            "Couldn't play \"{}\": ytcpp error: {}",
            m_session.playingVideo->video.id(),
            error.what()
        );
        errorOccured = true;
    }
    catch (const std::runtime_error& error)
    {
        m_logger.error(
            "Couldn't play \"{}\": Runtime error: {}",
            m_session.playingVideo->video.id(),
            error.what()
        );
        errorOccured = true;
    }
    catch (const std::exception& error)
    {
        m_logger.error(
            "Couldn't play \"{}\": Unknown error: {}",
            m_session.playingVideo->video.id(),
            error.what()
        );
        errorOccured = true;
    }
    catch (...)
    {
        m_logger.error(
            "Couldn't play \"{}\": Unknown error",
            m_session.playingVideo->video.id()
        );
        errorOccured = true;
    }

    std::lock_guard lock(m_mutex);
    dpp::discord_voice_client* client = getVoiceClient();
    if (!client)
    {
        m_threadStatus = ThreadStatus::Idle;
        return;
    }

    if (errorOccured)
    {
        Info info(m_session.guildId);
        m_root->message_create(info.settings().locale->playError(m_session.playingVideo->video).set_channel_id(m_session.textChannelId));
        client->insert_marker(Signal(Signal::Type::PlayError, videoId));
        m_threadStatus = ThreadStatus::Idle;
        return;
    }

    client->insert_marker(Signal(Signal::Type::Played, videoId));
    m_threadStatus = ThreadStatus::Idle;
}

dpp::discord_voice_client* Bot::Player::getVoiceClient()
{
    dpp::voiceconn* connection = m_client->get_voice(m_session.guildId);
    if (!connection || !connection->is_ready() || !connection->is_active())
        return nullptr;
    return connection->voiceclient;
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

void Bot::Player::signalReady(const Info& info)
{
    std::lock_guard lock(m_mutex);
    if (m_session.startTimestamp.is_not_a_date_time())
    {
        m_session.startTimestamp = pt::second_clock::local_time();
        if (m_timeout.enabled())
            m_timeout.reset();
        extractNextVideo(info);
        checkPlayingVideo();
        updateStatus(info);
    }
    else
    {
        checkPlayingVideo();
        m_root->message_create(info.settings().locale->reconnectedPlay(m_session.playingVideo->video).set_channel_id(m_session.textChannelId));
    }
}

void Bot::Player::signalMarker(const Signal& signal, Info& info)
{
    std::lock_guard lock(m_mutex);

/* Temporarily unsupported!
    if (signal.type() == Signal::Type::ChapterReached)
    {
        auto chapterEntry = std::find_if(
            m_session.playingVideo->video.chapters().begin(),
            m_session.playingVideo->video.chapters().end(),
            [signal](const ytcpp::Video::Chapter& chapter) { return chapter.name == signal.data(); }
        );
        if (chapterEntry != m_session.playingVideo->video.chapters().end())
            chapterReached(*chapterEntry, info);
        return;
    }
*/
    
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

void Bot::Player::updateVoiceServerEndpoint(const std::string& endpoint)
{
    std::lock_guard lock(m_mutex);
    m_session.voiceServerEndpoint = endpoint;
}

Bot::Session Bot::Player::session()
{
    std::lock_guard lock(m_mutex);
    return m_session;
}

void Bot::Player::addItem(const ytcpp::Item& item, const dpp::user& requester, const Info& info)
{
    std::unique_lock lock(m_mutex);
    if (!getVoiceClient())
    {
        m_session.queue.emplace_back(Session::EnqueuedItem{ item, requester });
        return;
    }

    if (m_session.playingVideo)
    {
        m_session.queue.emplace_back(Session::EnqueuedItem{ item, requester });
        return;
    }

    m_session.queue.emplace_back(Session::EnqueuedItem{ item, requester });
    extractNextVideo(info);
    updateStatus(info);
    startThread();
}

bool Bot::Player::paused()
{
    std::lock_guard lock(m_mutex);
    dpp::discord_voice_client* client = getVoiceClient();
    if (!client)
        return false;
    return client->is_paused();
}

bool Bot::Player::pauseResume(const Info& info)
{
    std::lock_guard lock(m_mutex);
    dpp::discord_voice_client* client = getVoiceClient();
    if (!client)
        return false;

    bool isPaused = !client->is_paused();
    client->pause_audio(isPaused);
    (isPaused ? m_timeout.enable() : m_timeout.disable());
    updateStatus(info);
    return isPaused;
}

void Bot::Player::seek(uint64_t timestamp, const Info& info)
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* client = getVoiceClient();
    if (!client)
        return;

/* Temporarily unsupported!
    if (!m_session.playingVideo->video.chapters().empty())
    {
        auto chapterEntry = DeduceChapter(m_session.playingVideo->video.chapters(), pt::time_duration(0, 0, timestamp));
        chapterReached(*chapterEntry, info);
    }
*/
    
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
    dpp::discord_voice_client* client = getVoiceClient();
    if (!client)
        return;

    stopThread(lock);
    client->stop_audio();
    incrementPlayedTracks(info);

    extractNextVideo(info);
    checkPlayingVideo();
    updateStatus(info);
}

void Bot::Player::skipPlaylist(Info& info)
{
    std::unique_lock lock(m_mutex);
    dpp::discord_voice_client* client = getVoiceClient();
    if (!client)
        return;

    stopThread(lock);
    client->stop_audio();
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
    dpp::discord_voice_client* client = getVoiceClient();
    if (!client)
        return;

    stopThread(lock);
    client->stop_audio();
    incrementPlayedTracks(info);

    m_session.playingVideo.reset();
    m_session.playingPlaylist.reset();
    m_session.queue.clear();
    m_timeout.enable();
    updateStatus(info);
}

void Bot::Player::endSession(Info& info, Locale::EndReason reason)
{
    bool clearVoiceStatus;
    switch (reason)
    {
        case Locale::EndReason::UserRequested:
        case Locale::EndReason::Timeout:
        {
            /*
            *   Bot is still sitting in the voice channel.
            *   Clearing voice status is necessary before leaving.
            */
            clearVoiceStatus = true;
            break;
        }
        case Locale::EndReason::EverybodyLeft:
        {
            /*
            *   Bot is the last user to leave the voice channel.
            *   Discord will clear voice status automatically.
            */
            clearVoiceStatus = false;
            break;
        }
        case Locale::EndReason::Kicked:
        case Locale::EndReason::Moved:
        {
            /*
            *   Bot is not in the voice channel already.
            *   Clearing voice status is implossible.
            */
            clearVoiceStatus = false;
            break;
        }
    }

    std::lock_guard lock(m_mutex);
    if (m_session.playingVideo)
        incrementPlayedTracks(info);

    dpp::discord_client* client = m_client;
    dpp::snowflake guildId = m_session.guildId;
    if (clearVoiceStatus)
        m_root->channel_set_voice_status(m_session.voiceChannelId, "", [client, guildId](const dpp::confirmation_callback_t&) { client->disconnect_voice(guildId); });
    else
        m_client->disconnect_voice(m_session.guildId);

    m_logger.info(
        "Session ended ({}) [{}, {} track{}]",
        Locale::EndReasonToString(reason),
        Utility::NiceString(pt::second_clock::local_time() - m_session.startTimestamp),
        m_session.tracksPlayed,
        LocaleEn::Cardinal(m_session.tracksPlayed)
    );

    if (reason != Locale::EndReason::UserRequested)
    {
        dpp::message message = info.settings().locale->sessionEnd(info.settings(), reason, m_session);
        m_root->message_create(message.set_channel_id(m_session.textChannelId));
    }
}

} // namespace kb
