#include "bot/error.hpp"

namespace kc {

const char* Bot::InitError::GetHint(Type type)
{
    switch (type)
    {
        case Type::CommandsConflict:
            return "Expected and registered version of slashcommands are different: try registering them";
        default:
            return "";
    }
}

Bot::InitError::InitError(Type type, const std::string& reason)
    : m_type(type)
    , m_reason(reason)
    , m_hint(GetHint(type))
{
    if (m_hint.empty())
        m_what = m_reason;
    else
        m_what = fmt::format("{} ({})", m_reason, m_hint);
}

} // namespace kc
