#pragma once

// STL modules
#include <string>

// Library Boost.Regex
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

namespace kc {

namespace Bot
{
    class Signal
    {
    public:
        enum class Type
        {
            PlayVideo,
            PlayPlaylist,
            RelatedSearch,
            SearchSelect,
            LivestreamSkipped,
            PremiereSkipped,
            Played,
            ChapterReached,
            PlayError,
            Unsupported,
            Unknown,
        };

    private:
        Type m_type;
        std::string m_data;

    public:
        /// @brief Create a signal
        /// @param type Signal type
        /// @param data Signal data
        Signal(Type type, const std::string& data);

        /// @brief Parse signal string
        /// @param signalString Signal string to parse
        Signal(const std::string& signalString);

        /// @brief Get signal type
        /// @return Signal type
        inline Type type() const
        {
            return m_type;
        }

        /// @brief Get signal data
        /// @return Signal data
        inline const std::string& data() const
        {
            return m_data;
        }

        /// @brief Convert signal to string
        /// @return Signal string
        inline operator std::string() const
        {
            return fmt::format("{}_{}", static_cast<int>(m_type), m_data);
        }
    };
}

} // namespace kc
