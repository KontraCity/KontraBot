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
    class Extractor
    {
    private:
        static constexpr int MaxAttempts = 5;
        static constexpr int FrameSize = 11520;

    public:
        class Frame : public std::vector<uint8_t>
        {
        private:
            bool m_timestampSet;
            uint64_t m_timestamp;

        public:
            /// @brief Create empty frame
            Frame();

            /// @brief Create a frame
            /// @param timestamp Frame timestamp
            Frame(double timestamp);

            /// @brief Clear frame
            void clear();

            /// @brief Check if frame's timestamp is set
            /// @return True if frame's timestamp is set
            inline bool timestampSet() const
            {
                return m_timestampSet;
            }

            /// @brief Get frame timestamp in milliseconds
            /// @return Frame timestamp in milliseconds
            inline uint64_t timestamp() const
            {
                return m_timestamp;
            }

            /// @brief Set frame timestamp in milliseconds
            /// @param timestamp The timestamp to set
            inline void setTimestamp(uint64_t timestamp)
            {
                m_timestamp = timestamp;
            }
        };

    private:
        static spdlog::logger Logger;

    public:
        std::string m_videoId;
        AVFormatContext* m_format;
        AVStream* m_stream;
        AVCodecContext* m_codec;
        SwrContext* m_resampler;
        int m_unitsPerSecond;
        uint64_t m_seekPosition;
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
        void seekTo(uint64_t timestamp);

        /// @brief Extract next audio frame
        /// @throw std::runtime_error if internal error occurs
        /// @return Audio frame: empty if all frames were extracted
        Frame extractFrame();
    };
}

} // namespace kc
