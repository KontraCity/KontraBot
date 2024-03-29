#pragma once

// STL modules
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

namespace kc {

namespace Bot
{
    class Timeout
    {
    public:
        // Callback function called when timeout occurs
        using Callback = std::function<void()>;

    private:
        bool m_enabled;
        Callback m_callback;
        uint32_t m_timeoutDuration;
        mutable std::mutex m_mutex;
        std::thread m_thread;
        std::condition_variable m_cv;

    private:
        /// @brief Timeout thread implementation
        void threadFunction();

    public:
        /// @brief Create timeout
        /// @param callback Callback function to call when timeout occurs
        /// @param timeoutDuration Timeout duration in seconds
        Timeout(const Callback& callback, uint32_t timeoutDuration);

        Timeout(const Timeout& other);

        ~Timeout();

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

} // namespace kc
