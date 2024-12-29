#include "bot/signal.hpp"

// Library Boost.Regex
#include <boost/regex.hpp>

// Library {fmt}
#include <fmt/format.h>

namespace kb {

static inline std::string CreateSignalString(Bot::Signal::Type type, const std::string& data)
{
    return fmt::format("{}_{}", static_cast<int>(type), data);
}

Bot::Signal::Signal(Type type, const std::string& data)
    : m_type(type)
    , m_data(data)
    , m_string(CreateSignalString(type, data))
{}

Bot::Signal::Signal(const std::string& signalString)
{
    boost::smatch matches;
    if (!boost::regex_match(signalString, matches, boost::regex(R"((\d+)_([\s\S]*))")))
    {
        if (boost::regex_match(signalString, matches, boost::regex(R"(^v?([^"&?\/\s]{11})0?$)")))
        {
            m_data = matches.str(1);
            m_type = Type::PlayVideo;
        }
        else if (boost::regex_match(signalString, matches, boost::regex(R"(^p?(PL[^"&?\/\s]{16,32}$|^OLAK5uy_[^"&?\/\s]{33})$)")))
        {
            m_data = matches.str(1);
            m_type = Type::PlayPlaylist;
        }
        else if (boost::regex_match(signalString, matches, boost::regex(R"(^related_([^"&?\/\s]{11})$)")))
        {
            m_data = matches.str(1);
            m_type = Type::RelatedSearch;
        }
        else if (
                signalString == "playpause" ||                                          // Unsupported "Play/Pause" button
                signalString == "next" ||                                               // Unsupported "Next track" button
                signalString == "stop" ||                                               // Unsupported "Stop" button
                signalString == "pause_resume" ||                                       // Unsupported "Pause"/"Resume" button
                signalString == "skip_video" ||                                         // Unsupported "Skip video" button
                signalString == "skip_playlist" ||                                      // Unsupported "Skip playlist" button
                signalString == "stop_playing" ||                                       // Unsupported "Stop playing" button
                signalString == "leave" ||                                              // Unsupported "Leave" button
                boost::regex_match(signalString, matches, boost::regex(R"(^\d+$)"))     // Unsupported "Select a chapter to seek!" dropdown
            )
        {
            m_data = "";
            m_type = Type::Unsupported;
        }
        else
        {
            m_data = "";
            m_type = Type::Unknown;
        }

        m_string = CreateSignalString(m_type, m_data);
        return;
    }
    m_data = matches.str(2);

    int signalId = std::stoi(matches.str(1));
    if (signalId < 0 || signalId >= static_cast<int>(Type::Unknown))
        return;
    m_type = static_cast<Type>(signalId);
    m_string = CreateSignalString(m_type, m_data);
}

} // namespace kb
