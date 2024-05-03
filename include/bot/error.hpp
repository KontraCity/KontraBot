#pragma once

// STL modules
#include <stdexcept>

// Library {fmt}
#include <fmt/format.h>

namespace kc {

namespace Bot
{
    class InitError : public std::exception
    {
    public:
        enum class Type
        {
            CouldntGetCommands,
            CommandsConflict,
        };

    private:
        /// @brief Get error hint
        /// @param type Type of error
        /// @return Error hint
        static const char* GetHint(Type type);

    private:
        Type m_type;
        std::string m_reason;
        std::string m_hint;
        std::string m_what;

    public:
        /// @brief Create bot initialization error
        /// @param type Error type
        /// @param reason Error reason
        InitError(Type type, const std::string& reason);

        /// @brief Get error type
        /// @return Error type
        inline Type type() const noexcept
        {
            return m_type;
        }

        /// @brief Get error reason
        /// @return Error reason
        inline const std::string& reason() const noexcept
        {
            return m_reason;
        }

        /// @brief Get error hint
        /// @return Error hint
        inline const std::string& hint() const noexcept
        {
            return m_hint;
        }

        /// @brief Get brief error description
        /// @return Brief error description
        inline const char* what() const noexcept
        {
            return m_what.c_str();
        }
    };
}

} // namespace kc
