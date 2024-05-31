#include "bot/signal.hpp"

namespace kc {

Bot::Signal::Signal(Type type, const std::string& data)
    : m_type(type)
    , m_data(data)
{}

Bot::Signal::Signal(const std::string& signalString)
    : m_type(Type::Unknown)
{
    boost::smatch matches;
    if (!boost::regex_match(signalString, matches, boost::regex(R"((\d+)_([\s\S]*))")))
    {
        if (boost::regex_match(signalString, matches, boost::regex(R"(^v?([^"&?\/\s]{11})$)")))
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
        return;
    }
    m_data = matches.str(2);

    int signalId = std::stoi(matches.str(1));
    if (signalId < 0 || signalId >= static_cast<int>(Type::Unknown))
        return;
    m_type = static_cast<Type>(signalId);
}

} // namespace kc
