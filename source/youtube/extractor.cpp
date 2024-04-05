#include "youtube/extractor.hpp"

namespace kc {

Youtube::Extractor::Frame::Frame()
    : m_timestamp(0)
{}

Youtube::Extractor::Frame::Frame(double timestamp)
    : m_timestamp(static_cast<uint64_t>(timestamp < 0 ? 0 : timestamp))
{}

spdlog::logger Youtube::Extractor::Logger("extractor", std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

Youtube::Extractor::Extractor(const std::string& videoId)
    : m_formatContext(nullptr)
    , m_stream(nullptr)
    , m_codecContext(nullptr)
    , m_unitsPerSecond(0)
    , m_seekPosition(0)
{
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
        Curl::Response playerResponse = Client::Instance->requestApi(Client::Type::Android, "player", { {"videoId", videoId} });
        if (playerResponse.code != 200)
        {
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Extractor::Extractor(): "
                "Couldn't get API response [video: \"{}\", client: \"web\", response code: {}]",
                videoId, playerResponse.code
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
                Logger.warn("Video \"{}\": API response is for wrong video (\"{}\"), trying \"tv_embedded\" client", videoId, responseVideoId);
                clientFallback = true;
            }

            if (clientFallback)
            {
                playerResponse = Client::Instance->requestApi(Client::Type::TvEmbedded, "player", { {"videoId", videoId} });
                if (playerResponse.code != 200)
                {
                    throw std::runtime_error(fmt::format(
                        "kc::Youtube::Extractor::Extractor(): Couldn't get API response [video: \"{}\", player: \"tv_embedded\", response code: {}]",
                        videoId, playerResponse.code
                    ));
                }

                playerResponseJson = json::parse(playerResponse.data);
                if (playerResponseJson["playabilityStatus"]["status"] != "OK")
                    throw YoutubeError(YoutubeError::Type::Unknown, videoId, "unknown error");
            }

            bool urlResolved = false;
            int bestBitrate = 0;
            for (const json& formatObject : playerResponseJson["streamingData"]["adaptiveFormats"])
            {
                /*
                *   We need an audio format that is:
                *       - A WebM container;
                *       - Opus encoded;
                *       - Contains 2 channels;
                *       - Has 48kHz sampling rate.
                */
                if (formatObject["mimeType"] != "audio/webm; codecs=\"opus\"")
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

            if (audioUrl.empty())
                throw LocalError(LocalError::Type::AudioNotSupported, videoId);
            if (!urlResolved)
                audioUrl = Youtube::Client::Instance->decryptSignatureCipher(audioUrl);
        }
        catch (const json::exception& error)
        {
            throw std::runtime_error(fmt::format(
                "kc::Youtube::Extractor::Extractor(): "
                "Couldn't parse API response JSON [video: \"{}\", id: {}]",
                videoId, error.id
            ));
        }

        int result = avformat_open_input(&m_formatContext, audioUrl.c_str(), nullptr, nullptr);
        if (result == 0)
            break;

        if (attempt == MaxAttempts)
            throw LocalError(LocalError::Type::CouldntDownload, videoId);
        Logger.warn("Video \"{}\": Attempt {}/{} failed, retrying", videoId, attempt, MaxAttempts);
    }

    int result = avformat_find_stream_info(m_formatContext, nullptr);
    if (result < 0)
    {
        avformat_close_input(&m_formatContext);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't find audio stream info [video: \"{}\", return code: {}]",
            videoId, result
        ));
    }

    int streamIndex = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIndex < 0)
    {
        avformat_close_input(&m_formatContext);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't find best audio stream [video: \"{}\", return code: {}]",
            videoId, result
        ));
    }

    m_stream = m_formatContext->streams[streamIndex];
    m_unitsPerSecond = static_cast<int>(static_cast<double>(m_stream->time_base.den) / m_stream->time_base.num);
    const AVCodec* decoder = avcodec_find_decoder(m_stream->codecpar->codec_id);
    if (!decoder)
    {
        avformat_close_input(&m_formatContext);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't find audio decoder [video: \"{}\"]",
            videoId
        ));
    }

    m_codecContext = avcodec_alloc_context3(decoder);
    if (!m_codecContext)
    {
        avformat_close_input(&m_formatContext);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't allocate codec context [video: \"{}\"]",
            videoId
        ));
    }

    result = avcodec_open2(m_codecContext, decoder, nullptr);
    if (result != 0)
    {
        avcodec_free_context(&m_codecContext);
        avformat_close_input(&m_formatContext);
        throw std::runtime_error(fmt::format(
            "kc::Youtube::Extractor::Extractor(): "
            "Couldn't open audio decoder context [video: \"{}\", return code: {}]",
            videoId, result
        ));
    }
}

Youtube::Extractor::~Extractor()
{
    avcodec_close(m_codecContext);
    avcodec_free_context(&m_codecContext);
    avformat_close_input(&m_formatContext);
}

void Youtube::Extractor::seekTo(uint64_t timestamp)
{
    m_seekPosition = timestamp * m_unitsPerSecond;
    av_seek_frame(m_formatContext, m_stream->index, m_seekPosition, AVSEEK_FLAG_BACKWARD);
}

Youtube::Extractor::Frame Youtube::Extractor::extractFrame()
{
    while (true)
    {
        AVPacket packet;
        if (av_read_frame(m_formatContext, &packet) < 0)
        {
            av_packet_unref(&packet);
            return {};
        }

        if (m_seekPosition != 0 && static_cast<uint64_t>(packet.dts) < m_seekPosition)
        {
            av_packet_unref(&packet);
            continue;
        }
        m_seekPosition = 0;

        Frame frame(static_cast<double>(packet.dts) / m_unitsPerSecond * 1'000);
        frame.insert(frame.begin(), packet.data, packet.data + packet.size);
        av_packet_unref(&packet);
        return frame;
    }
}

} // namespace kc
