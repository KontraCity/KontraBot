#pragma once

// STL modules
#include <string>
#include <string_view>

namespace kb {

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
        std::string m_string;

    public:
        /// @brief Create a signal
        /// @param type Signal type
        /// @param data Signal data
        Signal(Type type, const std::string& data);

        /// @brief Parse signal string
        /// @param signalString Signal string to parse
        Signal(const std::string& signalString);

    public:
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
            return m_string;
        }

        /// @brief Convert signal to string view
        /// @return Signal string view
        inline operator std::string_view() const
        {
            return { m_string.c_str() };
        }
    };
}

} // namespace kb
