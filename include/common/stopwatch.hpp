#pragma once

// STL modules
#include <chrono>

namespace kb {

class Stopwatch
{
private:
    using clock = std::chrono::high_resolution_clock;
    clock::time_point m_start;

public:
    /// @brief Initialize and start stopwatch
    Stopwatch()
        : m_start(clock::now())
    {}

public:
    /// @brief Reset stopwatch
    inline void reset()
    {
        m_start = clock::now();
    }

    /// @brief Get elapsed time in seconds
    /// @return Elapsed time in seconds
    inline float seconds() const
    {
        return milliseconds() / 1000.0f;
    }

    /// @brief Get elapsed time in milliseconds
    /// @return Elapsed time in milliseconds
    inline float milliseconds() const
    {
        return microseconds() / 1000.0f;
    }

    /// @brief Get elapsed time in microseconds
    /// @return Elapsed time in microseconds
    inline size_t microseconds() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - m_start).count();
    }
};

} // namespace kb
