#include "core/downloader.hpp"
using namespace kb::DownloaderConst;

// STL modules
#include <algorithm>
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
#include "ytcpp/format.hpp"
#include "ytcpp/utility.hpp"

namespace kb {

/* Namespace aliases and imports */
using nlohmann::json;

Downloader::Frame::Frame()
    : m_timestamp(-1)
{}

void Downloader::Frame::clear()
{
    m_timestamp = -1;
    vector::clear();
}

int Downloader::ProgressCallback(Downloader* target, double downloadTotal, double downloadNow, double uploadTotal, double uploadNow)
{
    std::lock_guard lock(target->m_mutex);
    return static_cast<int>(target->m_threadStatus == ThreadStatus::Stopped);
}

size_t Downloader::HeaderWriter(uint8_t* data, size_t itemSize, size_t itemCount, Downloader* target)
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

size_t Downloader::DownloaderWriter(uint8_t* data, size_t itemSize, size_t itemCount, Downloader* target)
{
    std::lock_guard lock(target->m_mutex);
    target->m_buffer.insert(target->m_buffer.end(), data, data + itemCount);
    target->m_cv.notify_all();
    return itemSize * itemCount;
}

int Downloader::Read(void* root, uint8_t* buffer, int bufferLength)
{
    Downloader* extractor = reinterpret_cast<Downloader*>(root);
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

int64_t Downloader::Seek(void* root, int64_t offset, int whence)
{
    Downloader* extractor = reinterpret_cast<Downloader*>(root);
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

Downloader::Downloader(const std::string& videoId)
    : m_logger(kb::Utility::CreateLogger(fmt::format("extractor \"{}\"", videoId)))
    , m_videoId(ytcpp::Utility::ExtractVideoId(videoId))
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

    ytcpp::Format::List formats(m_videoId);
    uint64_t bestBitrate = 0;
    for (const ytcpp::Format::Instance& format : formats) {
        if (format->type() != ytcpp::Format::Type::Audio)
            continue;

        if (format->bitrate() < bestBitrate)
            continue;
        bestBitrate = format->bitrate();
        m_audioUrl = format->url();
    }

    {
        std::unique_lock lock(m_mutex);
        startThread();
        m_cv.wait(lock);
    }

    if (m_threadStatus == ThreadStatus::Error) {
        stopThread();
        throw std::runtime_error("Download error");
    }

    m_io = avio_alloc_context(nullptr, 0, 0, this, &Downloader::Read, nullptr, &Downloader::Seek);
    if (!m_io)
    {
        stopThread();
        throw std::runtime_error(fmt::format(
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
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
            "kb::Downloader::Downloader(): "
            "Couldn't initialize resampler [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }
}

Downloader::~Downloader()
{
    swr_free(&m_resampler);
    avcodec_free_context(&m_codec);
    avformat_close_input(&m_format);
    avio_context_free(&m_io);
    stopThread();
}

void Downloader::threadFunction(uint64_t startPosition)
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

        if (Config::ProxyEnabled())
        {
            result = curl_easy_setopt(curl.get(), CURLOPT_PROXY, Config::ProxyUrl().c_str());
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

        result = curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, &Downloader::ProgressCallback);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request progress callback function [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't enable request progress callback [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, this);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request header target [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, &Downloader::HeaderWriter);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request header function [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, this);
        if (result != CURLE_OK)
            throw std::runtime_error(fmt::format("Couldn't configure request write target [return code: {}]", static_cast<int>(result)));

        result = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &Downloader::DownloaderWriter);
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

void Downloader::startThread(uint64_t startPosition)
{
    if (m_thread.joinable())
        m_thread.join();
    m_thread = std::thread(&Downloader::threadFunction, this, startPosition);
}

void Downloader::stopThread()
{
    m_threadStatus = ThreadStatus::Stopped;
    if (m_thread.joinable())
        m_thread.join();
}

void Downloader::seekTo(int64_t timestamp)
{
    m_seekPosition = timestamp * m_unitsPerSecond;
    av_seek_frame(m_format, m_stream->index, m_seekPosition, AVSEEK_FLAG_BACKWARD);
}

Downloader::Frame Downloader::extractFrame()
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
                "kb::Downloader::extractFrame(): "
                "Couldn't send packet [video: \"{}\", return code: {}]",
                m_videoId, result
            ));
        }

        AVFrame* frame = av_frame_alloc();
        if (!frame)
        {
            av_packet_unref(&packet);
            throw std::runtime_error(fmt::format(
                "kb::Downloader::extractFrame(): "
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
                    "kb::Downloader::extractFrame(): "
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
                    "kb::Downloader::extractFrame(): "
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
                    "kb::Downloader::extractFrame(): "
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
                    "kb::Downloader::extractFrame(): "
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
                        "kb::Downloader::extractFrame(): "
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
