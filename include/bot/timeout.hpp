#pragma once

// STL modules
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

namespace kb {

namespace Bot
{
    class Timeout
    {
    public:
        // Function called when timeout occurs
        using Callback = std::function<void()>;

    private:
        mutable std::mutex m_mutex;
        std::thread m_thread;
        std::condition_variable m_cv;
        uint64_t m_timeoutDuration;
        bool m_enabled;
        Callback m_callback;

    public:
        /// @brief Create timeout
        /// @param callback Callback function to call when timeout occurs
        /// @param timeoutDuration Timeout duration in seconds
        Timeout(const Callback& callback, uint64_t timeoutDuration);

        Timeout(const Timeout& other);

        ~Timeout();

    private:
        /// @brief Timeout thread implementation
        void threadFunction();

    public:
        /// @brief Set new timeout duration
        /// @param Timeout duration in seconds
        void setTimeoutDuration(uint64_t timeoutDuration);

        /// @brief Check if timeout is enabled
        /// @return True if enabled
        bool enabled() const;

        /// @brief Enable timeout
        void enable();

        /// @brief Disable timeout
        void disable();

        /// @brief Reset timeout timer
        void reset();
    };
}

} // namespace kb
