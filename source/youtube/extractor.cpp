#include "youtube/extractor.hpp"

namespace kc {

Youtube::Extractor::Extractor(const std::string& videoId)
{
    if (!boost::regex_match(videoId, boost::regex(Video::ValidateId)))
    {
        throw std::invalid_argument(fmt::format(
            "kc::Youtube::Extractor::Extractor(): [videoId]: \"{0}\": Not a valid video ID",
            videoId
        ));
    }

    for (int attempt = 1; ; ++attempt)
    {
        Curl::Response playerResponse = Client::Instance->requestApi(Client::Type::Android, "player", { {"videoId", videoId} });
        if (playerResponse.code != 200)
            throw std::runtime_error("kc::Youtube::Extractor::Extractor(): Couldn't get YouTube API response");

        std::string audioUrl;
        try
        {
            json playerResponseJson = json::parse(playerResponse.data);
            if (playerResponseJson["playabilityStatus"]["status"] != "OK")
            {
                playerResponse = Client::Instance->requestApi(Client::Type::TvEmbedded, "player", { {"videoId", videoId} });
                if (playerResponse.code != 200)
                    throw std::runtime_error("kc::Youtube::Extractor::Extractor(): Couldn't get YouTube API response");

                playerResponseJson = json::parse(playerResponse.data);
                if (playerResponseJson["playabilityStatus"]["status"] != "OK")
                    throw LocalError(LocalError::Type::UnknownYoutubeError, videoId);
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
                throw LocalError(LocalError::Type::FormatNotFound, videoId);

            if (!urlResolved)
                audioUrl = Youtube::Client::Instance->decryptSignatureCipher(audioUrl);
        }
        catch (const json::exception&)
        {
            throw std::runtime_error("kc::Youtube::Extractor::Extractor(): Couldn't parse YouTube API response JSON");
        }

        int result = avformat_open_input(&m_formatContext, audioUrl.c_str(), nullptr, nullptr);
        if (result == 0)
            break;

        if (attempt == MaxExtractionAttempts)
            throw LocalError(LocalError::Type::DownloadError, videoId);
        spdlog::warn("Extraction attempt {}/{} of video \"{}\" failed, retrying", attempt, MaxExtractionAttempts, videoId);
    }

    int result = avformat_find_stream_info(m_formatContext, nullptr);
    if (result < 0)
    {
        avformat_close_input(&m_formatContext);
        throw std::runtime_error("kc::Youtube::Extractor::Extractor(): Couldn't find audio stream info");
    }

    int streamIndex = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIndex < 0)
    {
        avformat_close_input(&m_formatContext);
        throw std::runtime_error("kc::Youtube::Extractor::Extractor(): Couldn't find best audio stream");
    }

    m_stream = m_formatContext->streams[streamIndex];
    m_unitsPerSecond = static_cast<int>(static_cast<double>(m_stream->time_base.den) / m_stream->time_base.num);
    const AVCodec* decoder = avcodec_find_decoder(m_stream->codecpar->codec_id);
    if (!decoder)
    {
        avformat_close_input(&m_formatContext);
        throw std::runtime_error("kc::Youtube::Extractor::Extractor(): Couldn't find audio decoder");
    }

    m_codecContext = avcodec_alloc_context3(decoder);
    if (!m_codecContext)
    {
        avformat_close_input(&m_formatContext);
        throw std::runtime_error("kc::Youtube::Extractor::Extractor(): Couldn't allocate codec context");
    }

    result = avcodec_open2(m_codecContext, decoder, nullptr);
    if (result != 0)
    {
        avcodec_free_context(&m_codecContext);
        avformat_close_input(&m_formatContext);
        throw std::runtime_error("kc::Youtube::Extractor::Extractor(): Couldn't open audio decoder context");
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

        double timestampMs = static_cast<double>(packet.dts) / m_unitsPerSecond * 1'000;
        Frame frame = { {packet.data, packet.data + packet.size}, static_cast<size_t>(timestampMs <= 0 ? 0 : timestampMs) };
        av_packet_unref(&packet);
        return frame;
    }
}

} // namespace kc
