#pragma once

// STL modules
#include <string>
#include <vector>
#include <stdexcept>

// Library nlohmann/json
#include <nlohmann/json.hpp>

// Library Boost.Regex
#include <boost/regex.hpp>

extern "C" {
    // FFmpeg libraries
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
}

// Library spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "youtube/client.hpp"
#include "youtube/error.hpp"
#include "youtube/video.hpp"

namespace kc {

/* Namespace aliases and imports */
using nlohmann::json;

namespace Youtube
{
    namespace ExtractorConst
    {
        // Maximum count of extraction attempts
        constexpr int MaxAttempts = 5;

        /*
         *  Size of audio frame for DPP. If the frame is smaller, the rest is filled with silence.
         *  All frames must be this size, and only the last frame can be smaller.
        */
        constexpr int FrameSize = 11520;

        // Output PCM data properties
        constexpr int OutputLayout = AV_CH_LAYOUT_STEREO;
        constexpr int OutputChannelCount = 2;
        constexpr AVSampleFormat OutputFormat = AV_SAMPLE_FMT_S16P;
        constexpr int OutputSampleRate = 48000;
    }

    class Extractor
    {
    public:
        class Frame : public std::vector<uint8_t>
        {
        private:
            int64_t m_timestamp;

        public:
            /// @brief Create empty frame
            Frame();

            /// @brief Clear frame
            void clear();

            /// @brief Get frame timestamp
            /// @return Frame timestamp in milliseconds
            inline int64_t timestamp() const
            {
                return m_timestamp;
            }

            /// @brief Set frame timestamp
            /// @param timestamp Frame timestamp in milliseconds
            inline void setTimestamp(int64_t timestamp)
            {
                if (timestamp < 0)
                    timestamp = -1;
                m_timestamp = timestamp;
            }
        };

    public:
        spdlog::logger m_logger;
        std::string m_videoId;
        AVFormatContext* m_format;
        AVStream* m_stream;
        AVCodecContext* m_codec;
        SwrContext* m_resampler;
        int m_unitsPerSecond;
        int64_t m_seekPosition;
        Frame m_overflowFrame;

    public:
        /// @brief Initialize audio extractor
        /// @param videoId ID of video to extract
        /// @throw std::invalid_argument if [videoId] is not a valid video ID
        /// @throw std::runtime_error if internal error occurs
        /// @throw kc::Youtube::YoutubeError if YouTube error occurs
        /// @throw kc::Youtube::LocalError if extraction error occurs
        Extractor(const std::string& videoId);

        ~Extractor();

        /// @brief Seek audio track
        /// @param timestamp Timestamp to seek to in seconds
        void seekTo(int64_t timestamp);

        /// @brief Extract next audio frame
        /// @throw std::runtime_error if internal error occurs
        /// @return Audio frame: empty if all frames were extracted
        Frame extractFrame();
    };
}

} // namespace kc
