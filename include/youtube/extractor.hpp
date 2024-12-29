#pragma once

// STL modules
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

extern "C" {
    // FFmpeg libraries
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
}

// Library spdlog
#include <spdlog/spdlog.h>

namespace kc {

namespace Youtube
{
    namespace ExtractorConst
    {
        constexpr int MaxExtractionAttempts = 5;    // Maximum count of extraction attempts
        constexpr int MaxRequestAttempts = 5;       // Maximum count of request attempts

        /*
         *  Size of audio frame for DPP. If the frame is smaller, the rest is filled with silence.
         *  All frames must be this size, and only the last frame can be smaller.
        */
        constexpr int FrameSize = 11520;

        // Output PCM data properties
        constexpr AVChannelLayout OutputChannelLayout = AV_CHANNEL_LAYOUT_STEREO;
        constexpr AVSampleFormat OutputFormat = AV_SAMPLE_FMT_S16P;
        constexpr int OutputSampleRate = 48000;
    }

    class Extractor
    {
    private:
        enum class ThreadStatus
        {
            Idle,
            Running,
            Stopped,
            Error,
        };

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

    private:
        /// @brief Curl progress callback
        /// @param target Extractor target
        /// @param downloadTotal Count of downloaded bytes
        /// @param downloadNow Count of total bytes to download
        /// @param uploadTotal Count of uploaded bytes
        /// @param uploadNow Count of total bytes to upload
        /// @return 1 if download should be terminated, 0 otherwise
        static int ProgressCallback(Extractor* target, double downloadTotal, double downloadNow, double uploadTotal, double uploadNow);

        /// @brief Curl header writer callback
        /// @param data Data to write
        /// @param itemSize Size of one item in bytes
        /// @param itemCount Count of items
        /// @param target Target to write data to
        /// @return Count of written bytes
        static size_t HeaderWriter(uint8_t* data, size_t itemSize, size_t itemCount, Extractor* target);

        /// @brief Curl extractor writer callback
        /// @param data Data to write
        /// @param itemSize Size of one item in bytes
        /// @param itemCount Count of items
        /// @param target Target to write data to
        /// @return Count of written bytes
        static size_t ExtractorWriter(uint8_t* data, size_t itemSize, size_t itemCount, Extractor* target);

        /// @brief FFmpeg data read callback
        /// @param root Extractor to read data from
        /// @param buffer Buffer to read data to
        /// @param bufferLength Length of the read buffer
        /// @return Count of bytes read
        static int Read(void* root, uint8_t* buffer, int bufferLength);

        /// @brief FFmpeg data seek callback
        /// @param root Extractor to seek data in
        /// @param offset Position offset in bytes
        /// @param whence Whence the offset is from
        /// @return Seeked data position
        static int64_t Seek(void* root, int64_t offset, int whence);

    public:
        spdlog::logger m_logger;
        std::string m_videoId;
        std::string m_audioUrl;
        uint64_t m_fileSize;

        std::mutex m_mutex;
        std::thread m_thread;
        ThreadStatus m_threadStatus;
        std::condition_variable m_cv;
        std::vector<uint8_t> m_buffer;
        uint64_t m_position;
        uint64_t m_positionOffset;

        AVIOContext* m_io;
        AVFormatContext* m_format;
        AVStream* m_stream;
        AVCodecContext* m_codec;
        SwrContext* m_resampler;
        int m_unitsPerSecond;
        int64_t m_seekPosition;
        Frame m_overflowFrame;

    private:
        /// @brief Download thread implementation
        /// @param startPosition Byte position to start download from
        void threadFunction(uint64_t startPosition);

        /// @brief Start download thread
        /// @param startPosition Byte position to start download from
        void startThread(uint64_t startPosition = 0);

        /// @brief Stop download thread
        void stopThread();

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
