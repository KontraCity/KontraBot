#include "youtube/extractor.hpp"

namespace kc {

Youtube::Extractor::Frame::Frame()
    : m_timestampSet(false)
    , m_timestamp(0)
{}

Youtube::Extractor::Frame::Frame(double timestamp)
    : m_timestampSet(true)
    , m_timestamp(static_cast<uint64_t>(timestamp < 0 ? 0 : timestamp))
{}

void Youtube::Extractor::Frame::clear()
{
    m_timestampSet = false;
    m_timestamp = 0;
    vector::clear();
}

spdlog::logger Youtube::Extractor::Logger("extractor", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

Youtube::Extractor::Extractor(const std::string& videoId)
    : m_videoId(videoId)
    , m_format(nullptr)
    , m_stream(nullptr)
    , m_codec(nullptr)
    , m_resampler(nullptr)
    , m_unitsPerSecond(0)
    , m_seekPosition(0)
{
    av_log_set_level(AV_LOG_QUIET);
    if (!boost::regex_match(videoId, boost::regex(VideoConst::ValidateId)))
    {
        throw std::invalid_argument(fmt::format(
            "kc::Youtube::Extractor::Extractor(): [videoId]: \"{}\": "
            "Not a valid video ID",
            videoId
        ));
    }

    for (int attempt = 1; ; ++attempt)
    {
        Curl::Response playerResponse = Client::Instance->requestApi(Client::Type::IOS, "player", { {"videoId", m_videoId} });
        if (playerResponse.code != 200)
        {
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Extractor::Extractor(): "
                "Couldn't get API response [video: \"{}\", client: \"ios\", response code: {}]",
                m_videoId, playerResponse.code
            ));
        }

        std::string audioUrl;
        try
        {
            json playerResponseJson = json::parse(playerResponse.data);
            bool clientFallback = (playerResponseJson["playabilityStatus"]["status"] != "OK");

            std::string responseVideoId = playerResponseJson["videoDetails"]["videoId"];
            if (responseVideoId != videoId)
            {
                Logger.warn("Video \"{}\": API response is for wrong video \"{}\", trying \"tv_embedded\" client", m_videoId, responseVideoId);
                clientFallback = true;
            }

            if (clientFallback)
            {
                playerResponse = Client::Instance->requestApi(Client::Type::TvEmbedded, "player", { {"videoId", m_videoId} });
                if (playerResponse.code != 200)
                {
                    throw std::runtime_error(fmt::format(
                        "kc::Youtube::Extractor::Extractor(): Couldn't get API response [video: \"{}\", player: \"tv_embedded\", response code: {}]",
                        m_videoId, playerResponse.code
                    ));
                }

                playerResponseJson = json::parse(playerResponse.data);
                if (playerResponseJson["playabilityStatus"]["status"] != "OK")
                    throw YoutubeError(YoutubeError::Type::Unknown, m_videoId, "unknown error");
            }

            bool urlResolved = false;
            int bestBitrate = 0;
            for (const json& formatObject : playerResponseJson["streamingData"]["adaptiveFormats"])
            {
                /*
                *   We primarily need an audio format that is:
                *       - A WebM container;
                *       - Opus encoded;
                *       - Contains 2 channels;
                *       - Has 48kHz sampling rate.
                */
                std::string mimeType = formatObject["mimeType"];
                if (mimeType != "audio/webm; codecs=\"opus\"")
                    continue;
                if (formatObject["audioChannels"] != 2)
                    continue;
                if (formatObject["audioSampleRate"] != "48000")
                    continue;

                int bitrate = formatObject["bitrate"];
                if (bestBitrate >= bitrate)
                    continue;
                bestBitrate = bitrate;

                if (formatObject.contains("url"))
                {
                    audioUrl = formatObject["url"];
                    urlResolved = true;
                }
                else
                {
                    audioUrl = formatObject["signatureCipher"];
                    urlResolved = false;
                }
            }

            std::string bestMimeType;
            if (audioUrl.empty())
            {
                for (const json& formatObject : playerResponseJson["streamingData"]["adaptiveFormats"])
                {
                    /*
                    *   No Opus audio track was found.
                    *   We'll look for other audio tracks.
                    */
                    std::string mimeType = formatObject["mimeType"];
                    if (mimeType.find("audio/") == std::string::npos)
                        continue;

                    int bitrate = formatObject["bitrate"];
                    if (bestBitrate >= bitrate)
                        continue;
                    bestMimeType = mimeType;
                    bestBitrate = bitrate;

                    if (formatObject.contains("url"))
                    {
                        audioUrl = formatObject["url"];
                        urlResolved = true;
                    }
                    else
                    {
                        audioUrl = formatObject["signatureCipher"];
                        urlResolved = false;
                    }
                }

                if (audioUrl.empty())
                    throw LocalError(LocalError::Type::AudioNotSupported, m_videoId);
                Logger.warn("Video \"{}\": No Opus track was found. Selecting [{}] instead", m_videoId, bestMimeType);
            }

            if (!urlResolved)
                audioUrl = Youtube::Client::Instance->decryptSignatureCipher(audioUrl);
        }
        catch (const json::exception& error)
        {
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Extractor::Extractor(): "
                "Couldn't parse API response JSON [video: \"{}\", id: {}]",
                m_videoId, error.id
            ));
        }

        int result = avformat_open_input(&m_format, audioUrl.c_str(), nullptr, nullptr);
        if (result == 0)
            break;

        if (attempt == MaxAttempts)
            throw LocalError(LocalError::Type::CouldntDownload, m_videoId);
        Logger.warn("Video \"{}\": Attempt {}/{} failed, retrying", m_videoId, attempt, MaxAttempts);
    }

    int result = avformat_find_stream_info(m_format, nullptr);
    if (result < 0)
    {
        avformat_close_input(&m_format);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't find audio stream info [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }

    int streamIndex = av_find_best_stream(m_format, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIndex < 0)
    {
        avformat_close_input(&m_format);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
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
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't find audio decoder [video: \"{}\"]",
            m_videoId
        ));
    }

    m_codec = avcodec_alloc_context3(decoder);
    if (!m_codec)
    {
        avformat_close_input(&m_format);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't allocate codec context [video: \"{}\"]",
            videoId
        ));
    }

    result = avcodec_parameters_to_context(m_codec, parameters);
    if (result < 0)
    {
        avcodec_free_context(&m_codec);
        avformat_close_input(&m_format);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't fill codec context with stream parameters [video: \"{}\", return code: {}]",
            videoId, result
        ));
    }

    result = avcodec_open2(m_codec, decoder, nullptr);
    if (result != 0)
    {
        avcodec_free_context(&m_codec);
        avformat_close_input(&m_format);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
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
    m_resampler = swr_alloc_set_opts(
        nullptr,
        AV_CH_LAYOUT_STEREO,
        AV_SAMPLE_FMT_S16P,
        48000,
        parameters->channel_layout,
        static_cast<AVSampleFormat>(parameters->format),
        parameters->sample_rate,
        0,
        nullptr
    );
    if (!m_resampler)
    {
        avcodec_free_context(&m_codec);
        avformat_close_input(&m_format);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't allocate resampler context [video: \"{}\"]",
            m_videoId
        ));
    }

    result = swr_init(m_resampler);
    if (result < 0)
    {
        swr_free(&m_resampler);
        avcodec_free_context(&m_codec);
        avformat_close_input(&m_format);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't initialize resampler [video: \"{}\", return code: {}]",
            m_videoId, result
        ));
    }
}

Youtube::Extractor::~Extractor()
{
    swr_free(&m_resampler);
    avcodec_close(m_codec);
    avcodec_free_context(&m_codec);
    avformat_close_input(&m_format);
}

void Youtube::Extractor::seekTo(uint64_t timestamp)
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

            if (m_seekPosition != 0 && static_cast<uint64_t>(packet.dts) < m_seekPosition)
            {
                av_packet_unref(&packet);
                continue;
            }

            m_seekPosition = 0;
            break;
        }

        if (!rawFrame.timestampSet())
            rawFrame.setTimestamp(static_cast<double>(packet.dts) / m_unitsPerSecond * 1'000);

        int result = avcodec_send_packet(m_codec, &packet);
        if (result < 0)
        {
            av_packet_unref(&packet);
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Extractor::extractFrame(): "
                "Couldn't send packet [video: \"{}\", return code: {}]",
                m_videoId, result
            ));
        }

        AVFrame* frame = av_frame_alloc();
        if (!frame)
        {
            av_packet_unref(&packet);
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Extractor::extractFrame(): "
                "Couldn't allocate frame [video: \"{}\"]",
                m_videoId
            ));
        }

        while (avcodec_receive_frame(m_codec, frame) >= 0)
        {
            uint8_t* buffer = nullptr;
            int bytesAllocated = av_samples_alloc(&buffer, nullptr, 2, frame->nb_samples, AV_SAMPLE_FMT_S16P, 0);
            if (bytesAllocated < 0)
            {
                av_frame_free(&frame);
                av_packet_unref(&packet);
                throw std::runtime_error(fmt::format(
                    "kc::Youtube::Extractor::extractFrame(): "
                    "Couldn't allocate samples buffer [video: \"{}\", return code: {}]",
                    m_videoId, bytesAllocated
                ));
            }

            int samplesConverted = swr_convert(m_resampler, &buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
            if (samplesConverted < 0)
            {
                av_freep(&buffer);
                av_frame_free(&frame);
                av_packet_unref(&packet);
                throw std::runtime_error(fmt::format(
                    "kc::Youtube::Extractor::extractFrame(): "
                    "Couldn't convert samples [video: \"{}\", return code: {}]",
                    m_videoId, samplesConverted
                ));
            }

            int bytesConverted = av_samples_get_buffer_size(nullptr, 2, samplesConverted, AV_SAMPLE_FMT_S16P, 0);
            if (bytesConverted < 0)
            {
                av_freep(&buffer);
                av_frame_free(&frame);
                av_packet_unref(&packet);
                throw std::runtime_error(fmt::format(
                    "kc::Youtube::Extractor::extractFrame(): "
                    "Couldn't get converted bytes count [video: \"{}\", return code: {}]",
                    m_videoId, bytesConverted
                ));
            }

            rawFrame.reserve(rawFrame.size() + bytesConverted);
            for (int sampleIndex = 0; sampleIndex < samplesConverted; ++sampleIndex)
            {
                uint8_t* leftChannelSample = buffer + sampleIndex * 2;
                rawFrame.insert(rawFrame.end(), leftChannelSample, leftChannelSample + 2);
                uint8_t* rightChannelSample = leftChannelSample + (bytesConverted / 2);
                rawFrame.insert(rawFrame.end(), rightChannelSample, rightChannelSample + 2);
            }

            samplesConverted = swr_convert(m_resampler, &buffer, frame->nb_samples, nullptr, 0);
            if (samplesConverted < 0)
            {
                av_freep(&buffer);
                av_frame_free(&frame);
                av_packet_unref(&packet);
                throw std::runtime_error(fmt::format(
                    "kc::Youtube::Extractor::extractFrame(): "
                    "Couldn't flush resampler [video: \"{}\", return code: {}]",
                    m_videoId, samplesConverted
                ));
            }
            else if (samplesConverted != 0)
            {
                bytesConverted = av_samples_get_buffer_size(nullptr, 2, samplesConverted, AV_SAMPLE_FMT_S16P, 0);
                if (bytesConverted < 0)
                {
                    av_freep(&buffer);
                    av_frame_free(&frame);
                    av_packet_unref(&packet);
                    throw std::runtime_error(fmt::format(
                        "kc::Youtube::Extractor::extractFrame(): "
                        "Couldn't get flushed bytes count [video: \"{}\", return code: {}]",
                        m_videoId, bytesConverted
                    ));
                }

                rawFrame.reserve(rawFrame.size() + bytesConverted);
                for (int sampleIndex = 0; sampleIndex < samplesConverted; ++sampleIndex)
                {
                    uint8_t* leftChannelSample = buffer + sampleIndex * 2;
                    rawFrame.insert(rawFrame.end(), leftChannelSample, leftChannelSample + 2);
                    uint8_t* rightChannelSample = leftChannelSample + (bytesConverted / 2);
                    rawFrame.insert(rawFrame.end(), rightChannelSample, rightChannelSample + 2);
                }
            }

            av_freep(&buffer);
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

} // namespace kc
