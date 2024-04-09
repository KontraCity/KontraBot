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
        return;
    m_data = matches.str(2);

    int signalId = std::stoi(matches.str(1));
    if (signalId < 0 || signalId >= static_cast<int>(Type::Unknown))
        return;
    m_type = static_cast<Type>(signalId);
}

} // namespace kc
