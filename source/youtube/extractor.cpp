#include "youtube/extractor.hpp"
using namespace kb::Youtube::ExtractorConst;

// STL modules
#include <stdexcept>

// Library Boost.Regex
#include <boost/regex.hpp>

// Library nlohmann/json
#include <nlohmann/json.hpp>

// Library Curl
#include <curl/curl.h>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "core/config.hpp"
#include "core/utility.hpp"
#include "youtube/client.hpp"
#include "youtube/error.hpp"
#include "youtube/video.hpp"

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

Youtube::Extractor::Frame::Frame()
    : m_timestamp(-1)
{}

void Youtube::Extractor::Frame::clear()
{
    m_timestamp = -1;
    vector::clear();
}

int Youtube::Extractor::ProgressCallback(Extractor* target, double downloadTotal, double downloadNow, double uploadTotal, double uploadNow)
{
    std::lock_guard lock(target->m_mutex);
    return static_cast<int>(target->m_threadStatus == ThreadStatus::Stopped);
}

size_t Youtube::Extractor::HeaderWriter(uint8_t* data, size_t itemSize, size_t itemCount, Extractor* target)
{
    std::lock_guard lock(target->m_mutex);
    if (!target->m_fileSize)
    {
        std::string string(reinterpret_cast<char*>(data), itemCount);
        boost::smatch matches;
        if (boost::regex_search(string, matches, boost::regex(R"([Cc]ontent-[Ll]ength: (\d+))")))
            target->m_fileSize = std::stoull(matches.str(1));
    }
    return itemSize * itemCount;
}

size_t Youtube::Extractor::ExtractorWriter(uint8_t* data, size_t itemSize, size_t itemCount, Extractor* target)
{
    std::lock_guard lock(target->m_mutex);
    target->m_buffer.insert(target->m_buffer.end(), data, data + itemCount);
    target->m_cv.notify_all();
    return itemSize * itemCount;
}

int Youtube::Extractor::Read(void* root, uint8_t* buffer, int bufferLength)
{
    Extractor* extractor = reinterpret_cast<Extractor*>(root);
    std::unique_lock lock(extractor->m_mutex);

    while (true)
    {
        int bytesAvailable = static_cast<int>(extractor->m_buffer.size() + extractor->m_positionOffset - extractor->m_position);
        if (bytesAvailable < bufferLength)
        {
            if (extractor->m_threadStatus != ThreadStatus::Running)
            {
                if (bytesAvailable == 0)
                    return AVERROR_EOF;
            }
            else
            {
                extractor->m_cv.wait(lock);
                continue;
            }
        }

        // More bytes may be available than requested
        if (bytesAvailable > bufferLength)
            bytesAvailable = bufferLength;

        uint8_t* data = extractor->m_buffer.data() + (extractor->m_position - extractor->m_positionOffset);
        std::copy(data, data + bytesAvailable, buffer);
        extractor->m_position += bytesAvailable;
        return bytesAvailable;
    }
}

int64_t Youtube::Extractor::Seek(void* root, int64_t offset, int whence)
{
    Extractor* extractor = reinterpret_cast<Extractor*>(root);
    std::unique_lock lock(extractor->m_mutex);

    if (whence == AVSEEK_SIZE)
        return extractor->m_fileSize;
    else if (whence != SEEK_SET)
        return AVERROR(EINVAL);

    if (static_cast<size_t>(offset) <= extractor->m_position && extractor->m_positionOffset == 0)
    {
        extractor->m_position = offset;
        return extractor->m_position;
    }

    lock.unlock();
    extractor->stopThread();
    lock.lock();

    extractor->m_buffer.clear();
    extractor->m_position = offset;
    extractor->m_positionOffset = offset;
    extractor->startThread(offset);
    extractor->m_cv.wait(lock);
    return extractor->m_position;
}

Youtube::Extractor::Extractor(const std::string& videoId)
    : m_logger(kb::Utility::CreateLogger(fmt::format("extractor \"{}\"", videoId)))
    , m_videoId(videoId)
    , m_fileSize(0)
    , m_threadStatus(ThreadStatus::Idle)
    , m_position(0)
    , m_positionOffset(0)
    , m_io(nullptr)
    , m_format(nullptr)
    , m_stream(nullptr)
    , m_codec(nullptr)
    , m_resampler(nullptr)
    , m_unitsPerSecond(0)
    , m_seekPosition(0)
{
    av_log_set_callback([](void* opaque, int level, const char* format, va_list arguments)
    {
        static std::mutex mutex;
        std::lock_guard lock(mutex);

        /*
            *  The format string provided by ffmpeg libraries contains '\n' character at the end.
            *  The same character is inserted by spdlog, so the one in format string should be discarded.
        */
        std::string string(1024, '\0');
        int stringLength = vsnprintf(string.data(), string.size(), format, arguments);
        string.resize(stringLength - 1);

        static spdlog::logger logger = kb::Utility::CreateLogger("ffmpeg");
        switch (level)
        {
            case AV_LOG_INFO:
            {
                logger.info(string);
                break;
            }
            case AV_LOG_WARNING:
            {
                logger.warn(string);
                break;
            }
            case AV_LOG_ERROR:
            {
                logger.error(string);
                break;
            }
            case AV_LOG_FATAL:
            case AV_LOG_PANIC:
            {
                logger.critical(string);
                break;
            }
            default:
            {
                break;
            }
        }
    });

    if (!boost::regex_match(videoId, boost::regex(VideoConst::ValidateId)))
    {
        throw std::invalid_argument(fmt::format(
            "kb::Youtube::Extractor::Extractor(): [videoId]: \"{}\": "
            "Not a valid video ID",
            videoId
        ));
    }

    for (int attempt = 1; true; ++attempt)
    {
        Curl::Response playerResponse = Client::Instance->requestApi(Client::Type::IOS, "player", { {"videoId", m_videoId} });
        if (playerResponse.code != 200)
        {
            throw std::runtime_error(fmt::format(
                "kb::Youtube::Extractor::Extractor(): "
                "Couldn't get API response [video: \"{}\", client: \"ios\", response code: {}]",
                m_videoId, playerResponse.code
            ));
        }

        try
        {
            json playerResponseJson = json::parse(playerResponse.data);
            bool clientFallback = (playerResponseJson.at("playabilityStatus").at("status") != "OK");

            std::string responseVideoId = playerResponseJson.at("videoDetails").at("videoId");
            if (responseVideoId != videoId)
            {
                m_logger.warn("API response is for wrong video \"{}\", trying \"tv_embedded\" client", responseVideoId);
                clientFallback = true;
            }

            if (clientFallback)
            {
                playerResponse = Client::Instance->requestApi(Client::Type::TvEmbedded, "player", { {"videoId", m_videoId} });
                if (playerResponse.code != 200)
                {
                    throw std::runtime_error(fmt::format(
                        "kb::Youtube::Extractor::Extractor(): Couldn't get API response [video: \"{}\", player: \"tv_embedded\", response code: {}]",
                        m_videoId, playerResponse.code
                    ));
                }

                playerResponseJson = json::parse(playerResponse.data);
                if (playerResponseJson.at("playabilityStatus").at("status") != "OK")
                    throw YoutubeError(YoutubeError::Type::Unknown, m_videoId, "unknown error");
            }

            bool urlResolved = false;
            int bestBitrate = 0;
            std::string bestMimeType;
            for (const json& formatObject : playerResponseJson.at("streamingData").at("adaptiveFormats"))
            {
                std::string mimeType = formatObject.at("mimeType");
                if (mimeType.find("audio/") == std::string::npos)
                    continue;

                int bitrate = formatObject.at("bitrate");
                if (bitrate < bestBitrate)
                    continue;
                bestBitrate = bitrate;
                bestMimeType = mimeType;

                if (formatObject.contains("url"))
                {
                    m_audioUrl = formatObject.at("url");
                    urlResolved = true;
                }
                else
                {
                    m_audioUrl = formatObject.at("signatureCipher");
                    urlResolved = false;
                }
            }

            if (m_audioUrl.empty())
                throw LocalError(LocalError::Type::AudioNotSupported, m_videoId);
            if (!urlResolved)
                m_audioUrl = Youtube::Client::Instance->decryptSignatureCipher(m_audioUrl);
        }
        catch (const json::exception& error)
        {
            throw std::runtime_error(fmt::format(
                "kb::Youtube::Extractor::Extractor(): "
                "Couldn't parse API response JSON [video: \"{}\", id: {}]",
                m_videoId, error.id
            ));
        }

        {
            std::unique_lock lock(m_mutex);
            startThread();
            m_cv.wait(lock);
        }

        if (m_threadStatus != ThreadStatus::Error)
            break;
        stopThread();

        if (attempt == MaxExtractionAttempts)
            throw LocalError(LocalError::Type::CouldntDownload, m_videoId);
        m_logger.warn("Extraction attempt {}/{} failed, retrying", attempt, MaxExtractionAttempts);
    }

    m_io = avio_alloc_context(nullptr, 0, 0, this, &Extractor::Read, nullptr, &Extractor::Seek);
    if (!m_io)
    {
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't allocate IO context [video: \"{}\"]",
            m_videoId
        ));
    }

    m_format = avformat_alloc_context();
    if (!m_format)
    {
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't allocate format context [video: \"{}\"]",
            m_videoId
        ));
    }
    m_format->pb = m_io;

    int result = avformat_open_input(&m_format, "", nullptr, nullptr);
    if (result < 0)
    {
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't open input [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }

    result = avformat_find_stream_info(m_format, nullptr);
    if (result < 0)
    {
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't find audio stream info [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }

    int streamIndex = av_find_best_stream(m_format, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIndex < 0)
    {
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't find best audio stream [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }

    m_stream = m_format->streams[streamIndex];
    AVCodecParameters* parameters = m_stream->codecpar;

    m_unitsPerSecond = static_cast<int>(static_cast<double>(m_stream->time_base.den) / m_stream->time_base.num);
    const AVCodec* decoder = avcodec_find_decoder(m_stream->codecpar->codec_id);
    if (!decoder)
    {
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't find audio decoder [video: \"{}\"]",
            m_videoId
        ));
    }

    m_codec = avcodec_alloc_context3(decoder);
    if (!m_codec)
    {
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't allocate codec context [video: \"{}\"]",
            videoId
        ));
    }

    result = avcodec_parameters_to_context(m_codec, parameters);
    if (result < 0)
    {
        avcodec_free_context(&m_codec);
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't fill codec context with stream parameters [video: \"{}\", return code: {}]",
            videoId, result
        ));
    }

    m_codec->pkt_timebase = m_stream->time_base;
    result = avcodec_open2(m_codec, decoder, nullptr);
    if (result < 0)
    {
        avcodec_free_context(&m_codec);
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't open audio decoder context [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }

    /*
    *   DPP expects PCM data that:
    *       - Is interleaved stereo;
    *       - Has 16 bit depth;
    *       - Has 48kHz sample rate.
    */
    AVChannelLayout outputLayout = OutputChannelLayout;
    result = swr_alloc_set_opts2(
        &m_resampler,
        &outputLayout,
        OutputFormat,
        OutputSampleRate,
        &parameters->ch_layout,
        static_cast<AVSampleFormat>(parameters->format),
        parameters->sample_rate,
        0, nullptr
    );
    if (result < 0)
    {
        avcodec_free_context(&m_codec);
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't allocate resampler context [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }

    result = swr_init(m_resampler);
    if (result < 0)
    {
        swr_free(&m_resampler);
        avcodec_free_context(&m_codec);
        avformat_close_input(&m_format);
        avio_context_free(&m_io);
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Youtube::Extractor::Extractor(): "
            "Couldn't initialize resampler [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }
}

Youtube::Extractor::~Extractor()
{
    swr_free(&m_resampler);
    avcodec_free_context(&m_codec);
    avformat_close_input(&m_format);
    avio_context_free(&m_io);
    stopThread();
}

void Youtube::Extractor::threadFunction(uint64_t startPosition)
{
    {
        std::lock_guard lock(m_mutex);
        m_threadStatus = ThreadStatus::Running;
    }

    try
    {
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
        if (!curl.get())
            throw std::runtime_error("Couldn't initialize Curl");

        CURLcode result = curl_easy_setopt(curl.get(), CURLOPT_URL, m_audioUrl.c_str());
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request URL [return code: {}]", static_cast<int>(result)));

        if (Config::Instance->proxyEnabled())
        {
            result = curl_easy_setopt(curl.get(), CURLOPT_PROXY, Config::Instance->proxyUrl().c_str());
            if (result != CURLE_OK)
                throw std::runtime_error(fmt::format("Couldn't configure request proxy [return code: {}]", static_cast<int>(result)));
        }

        result = curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_LIMIT, 15360);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request low speed limit [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_TIME, 5);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request low speed timeout [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request redirection [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, this);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request progress callback target [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, &Extractor::ProgressCallback);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request progress callback function [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't enable request progress callback [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, this);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request header target [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, &Extractor::HeaderWriter);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request header function [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, this);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request write target [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &Extractor::ExtractorWriter);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request write function [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_RANGE, fmt::format("{}-", startPosition).c_str());
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request range [return code: {}]", static_cast<int>(result)));

        m_logger.info("Starting download from position {}", startPosition);
        for (int requestAttempt = 1, downloadAttempt = 1; true;)
        {
            result = curl_easy_perform(curl.get());
            if (result == CURLE_OK)
                break;
            else if (result == CURLE_ABORTED_BY_CALLBACK)
                return; // Thread is cancelled

            if (requestAttempt == MaxRequestAttempts)
            {
                m_logger.error("All {} request attempts failed (return code: {})", MaxRequestAttempts, static_cast<int>(result));
                throw std::runtime_error(fmt::format(
                    "Couldn't perform request in {} attempts [return code: {}]",
                    MaxRequestAttempts,
                    static_cast<int>(result))
                );
            }

            startPosition = m_positionOffset + m_buffer.size();
            if (result == CURLE_OPERATION_TIMEDOUT || result == CURLE_RECV_ERROR)
            {
                m_logger.warn(
                    "Download attempt #{} failed, retrying at position {}",
                    downloadAttempt++,
                    startPosition
                );
            }
            else
            {
                m_logger.warn(
                    "Request attempt #{}/{} failed (return code: {}), retrying at position {}",
                    requestAttempt++,
                    MaxRequestAttempts,
                    static_cast<int>(result),
                    startPosition
                );
            }

            result = curl_easy_setopt(curl.get(), CURLOPT_RANGE, fmt::format("{}-", startPosition).c_str());
            if (result != CURLE_OK)
                throw std::runtime_error(fmt::format("Couldn't configure request range [return code: {}]", static_cast<int>(result)));
        }

        long responseCode = 0;
        result = curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &responseCode);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't retrieve response code [return code: {}]", static_cast<int>(result)));

        /*
         * 200 = OK: whole file successfully downloaded
         * 206 = Partial Content: whole range successfully downloaded
        */
        if (responseCode != 200 && responseCode != 206)
            throw std::runtime_error(fmt::format("Couldn't initiate download [HTTP response code: {}]", responseCode));

        std::lock_guard lock(m_mutex);
        m_threadStatus = ThreadStatus::Idle;
        m_cv.notify_all();
        if (m_buffer.size() == m_fileSize)
            m_logger.info("Download finished successfully (total: {})", m_fileSize);
        else
            m_logger.info("Download finished successfully (current/total: {}/{})", m_buffer.size(), m_fileSize);
    }
    catch (const std::runtime_error& error)
    {
        std::lock_guard lock(m_mutex);
        m_threadStatus = ThreadStatus::Error;
        m_cv.notify_all();
        m_logger.error("Download error: {}", error.what());
    }
}

void Youtube::Extractor::startThread(uint64_t startPosition)
{
    if (m_thread.joinable())
        m_thread.join();
    m_thread = std::thread(&Extractor::threadFunction, this, startPosition);
}

void Youtube::Extractor::stopThread()
{
    m_threadStatus = ThreadStatus::Stopped;
    if (m_thread.joinable())
        m_thread.join();
}

void Youtube::Extractor::seekTo(int64_t timestamp)
{
    m_seekPosition = timestamp * m_unitsPerSecond;
    av_seek_frame(m_format, m_stream->index, m_seekPosition, AVSEEK_FLAG_BACKWARD);
}

Youtube::Extractor::Frame Youtube::Extractor::extractFrame()
{
    Frame rawFrame = m_overflowFrame;
    m_overflowFrame.clear();

    while (rawFrame.size() < FrameSize)
    {
        AVPacket packet;
        while (true)
        {
            if (av_read_frame(m_format, &packet) < 0)
            {
                av_packet_unref(&packet);
                return rawFrame;
            }

            if (m_seekPosition != 0 && packet.dts < m_seekPosition)
            {
                av_packet_unref(&packet);
                continue;
            }

            m_seekPosition = 0;
            break;
        }

        if (rawFrame.timestamp() == -1)
            rawFrame.setTimestamp(static_cast<int64_t>(static_cast<double>(packet.dts) / m_unitsPerSecond * 1'000));

        int result = avcodec_send_packet(m_codec, &packet);
        if (result < 0)
        {
            av_packet_unref(&packet);
            throw std::runtime_error(fmt::format(
                "kb::Youtube::Extractor::extractFrame(): "
                "Couldn't send packet [video: \"{}\", return code: {}]",
                m_videoId, result
            ));
        }

        AVFrame* frame = av_frame_alloc();
        if (!frame)
        {
            av_packet_unref(&packet);
            throw std::runtime_error(fmt::format(
                "kb::Youtube::Extractor::extractFrame(): "
                "Couldn't allocate frame [video: \"{}\"]",
                m_videoId
            ));
        }

        while (avcodec_receive_frame(m_codec, frame) >= 0)
        {
            uint8_t** buffer = nullptr;
            int bytesAllocated = av_samples_alloc_array_and_samples(&buffer, nullptr, OutputChannelLayout.nb_channels, frame->nb_samples, OutputFormat, 0);
            if (bytesAllocated < 0)
            {
                av_frame_free(&frame);
                av_packet_unref(&packet);
                throw std::runtime_error(fmt::format(
                    "kb::Youtube::Extractor::extractFrame(): "
                    "Couldn't allocate samples buffer [video: \"{}\", return code: {}]",
                    m_videoId, bytesAllocated
                ));
            }

            int samplesConverted = swr_convert(m_resampler, buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
            if (samplesConverted < 0)
            {
                av_freep(&buffer[0]);
                av_frame_free(&frame);
                av_packet_unref(&packet);
                throw std::runtime_error(fmt::format(
                    "kb::Youtube::Extractor::extractFrame(): "
                    "Couldn't convert samples [video: \"{}\", return code: {}]",
                    m_videoId, samplesConverted
                ));
            }

            int bytesConverted = av_samples_get_buffer_size(nullptr, OutputChannelLayout.nb_channels, samplesConverted, OutputFormat, 0);
            if (bytesConverted < 0)
            {
                av_freep(&buffer[0]);
                av_frame_free(&frame);
                av_packet_unref(&packet);
                throw std::runtime_error(fmt::format(
                    "kb::Youtube::Extractor::extractFrame(): "
                    "Couldn't get converted bytes count [video: \"{}\", return code: {}]",
                    m_videoId, bytesConverted
                ));
            }

            rawFrame.reserve(rawFrame.size() + bytesConverted);
            for (int sampleIndex = 0; sampleIndex < samplesConverted; ++sampleIndex)
            {
                uint8_t* leftChannelSample = buffer[0] + sampleIndex * 2;
                rawFrame.insert(rawFrame.end(), leftChannelSample, leftChannelSample + 2);
                uint8_t* rightChannelSample = buffer[1] + sampleIndex * 2;
                rawFrame.insert(rawFrame.end(), rightChannelSample, rightChannelSample + 2);
            }

            samplesConverted = swr_convert(m_resampler, buffer, frame->nb_samples, nullptr, 0);
            if (samplesConverted < 0)
            {
                av_freep(&buffer[0]);
                av_frame_free(&frame);
                av_packet_unref(&packet);
                throw std::runtime_error(fmt::format(
                    "kb::Youtube::Extractor::extractFrame(): "
                    "Couldn't flush resampler [video: \"{}\", return code: {}]",
                    m_videoId, samplesConverted
                ));
            }
            else if (samplesConverted != 0)
            {
                int bytesConverted = av_samples_get_buffer_size(nullptr, OutputChannelLayout.nb_channels, samplesConverted, OutputFormat, 0);
                if (bytesConverted < 0)
                {
                    av_freep(&buffer[0]);
                    av_frame_free(&frame);
                    av_packet_unref(&packet);
                    throw std::runtime_error(fmt::format(
                        "kb::Youtube::Extractor::extractFrame(): "
                        "Couldn't get converted bytes count [video: \"{}\", return code: {}]",
                        m_videoId, bytesConverted
                    ));
                }

                rawFrame.reserve(rawFrame.size() + bytesConverted);
                for (int sampleIndex = 0; sampleIndex < samplesConverted; ++sampleIndex)
                {
                    uint8_t* leftChannelSample = buffer[0] + sampleIndex * 2;
                    rawFrame.insert(rawFrame.end(), leftChannelSample, leftChannelSample + 2);
                    uint8_t* rightChannelSample = buffer[1] + sampleIndex * 2;
                    rawFrame.insert(rawFrame.end(), rightChannelSample, rightChannelSample + 2);
                }
            }

            av_freep(&buffer[0]);
        }

        av_frame_free(&frame);
        av_packet_unref(&packet);
    }

    if (rawFrame.size() > FrameSize)
    {
        m_overflowFrame.setTimestamp(rawFrame.timestamp() + 60);
        m_overflowFrame.insert(m_overflowFrame.end(), rawFrame.begin() + FrameSize, rawFrame.end());
        rawFrame.erase(rawFrame.begin() + FrameSize, rawFrame.end());
    }
    return rawFrame;
}

} // namespace kb
