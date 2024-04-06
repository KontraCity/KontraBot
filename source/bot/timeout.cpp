#include "bot/timeout.hpp"

namespace kc {

void Bot::Timeout::threadFunction()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    bool timeouted = !m_cv.wait_for(lock, std::chrono::seconds(m_timeoutDuration), [this]() { return !m_enabled; });
    m_enabled = false;

    if (timeouted)
        m_callback();
}

Bot::Timeout::Timeout(const Callback& callback, uint32_t timeoutDuration)
    : m_enabled(true)
    , m_callback(callback)
    , m_timeoutDuration(timeoutDuration)
{
    m_thread = std::thread(&Timeout::threadFunction, this);
}

Bot::Timeout::Timeout(const Timeout& other)
    : Timeout(other.m_callback, other.m_timeoutDuration)
{}

Bot::Timeout::~Timeout()
{
    disable();
    if (m_thread.joinable())
        m_thread.join();
}

bool Bot::Timeout::enabled() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_enabled;
}

void Bot::Timeout::enable()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_enabled)
        return;

    if (m_thread.joinable())
        m_thread.join();

    m_enabled = true;
    m_thread = std::thread(&Timeout::threadFunction, this);
}

void Bot::Timeout::disable()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_enabled)
            return;

        m_enabled = false;
        m_cv.notify_one();
    }

    if (m_thread.joinable())
        m_thread.join();
}

void Bot::Timeout::reset()
{
    disable();
    enable();
}

} // namespace kc
