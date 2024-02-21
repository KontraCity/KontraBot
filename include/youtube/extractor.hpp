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

// Library {fmt}
#include <fmt/format.h>

// Library spdlog
#include <spdlog/spdlog.h>

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
        // Count of maximum request and extraction attempts
        constexpr int MaxAttempts = 5;
    }

    class Extractor
    {
    public:
        struct Frame
        {
            std::vector<uint8_t> data;  // Frame data
            uint64_t timestamp;         // Frame timestamp from start in milliseconds
        };

    public:
        AVFormatContext* m_formatContext = nullptr;
        AVStream* m_stream = nullptr;
        AVCodecContext* m_codecContext = nullptr;
        int m_unitsPerSecond = 0;
        uint64_t m_seekPosition = 0;

    public:
        /// @brief Initialize audio extractor for video
        /// @param videoId ID of video to initialize for
        /// @throw std::invalid_argument if [videoId] is not a valid video ID
        /// @throw std::runtime_error if internal error occurs
        /// @throw kc::Youtube::Error if extraction fails
        Extractor(const std::string& videoId);

        ~Extractor();

        /// @brief Seek audio
        /// @param timestamp Timestamp to seek to in seconds
        void seekTo(uint64_t timestamp);

        /// @brief Extract next audio frame
        /// @return Audio frame: empty if all frames were extracted
        Frame extractFrame();
    };
}

} // namespace kc
