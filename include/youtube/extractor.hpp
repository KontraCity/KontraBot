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
}

// Library spdlog
#include <spdlog/spdlog.h>

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
        // Count of maximum extraction attempts
        static constexpr int MaxAttempts = 5;

    public:
        class Frame : public std::vector<uint8_t>
        {
        private:
            uint64_t m_timestamp;

        public:
            /// @brief Create empty frame
            Frame();

            /// @brief Create a frame
            /// @param timestamp Frame timestamp
            Frame(double timestamp);

            /// @brief Get frame timestamp in milliseconds
            /// @return Frame timestamp in milliseconds
            inline uint64_t timestamp() const
            {
                return m_timestamp;
            }
        };

    private:
        static spdlog::logger Logger;

    public:
        AVFormatContext* m_formatContext;
        AVStream* m_stream;
        AVCodecContext* m_codecContext;
        int m_unitsPerSecond;
        uint64_t m_seekPosition;

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
        /// @return Audio frame: empty if all frames were extracted
        Frame extractFrame();
    };
}

} // namespace kc
