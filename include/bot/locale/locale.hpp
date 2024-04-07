#pragma once

// STL modules
#include <memory>

// Library DPP
#include <dpp/dpp.h>

namespace kc {

namespace Bot
{
    namespace LocaleConst
    {
        namespace Statuses
        {
            namespace Success
            {
                constexpr uint32_t Color = dpp::colors::green;
                constexpr const char* Prefix = u8"✅";
            }

            namespace Question
            {
                constexpr uint32_t Color = dpp::colors::yellow;
                constexpr const char* Prefix = u8"❔";
            }

            namespace Problem
            {
                constexpr uint32_t Color = dpp::colors::red;
                constexpr const char* Prefix = u8"❌";
            }

            namespace Error
            {
                constexpr uint32_t Color = dpp::colors::dark_red;
                constexpr const char* Prefix = u8"💀";
            }
        }
    }

    class Locale
    {
    public:
        // Locale instance pointer
        using Pointer = std::unique_ptr<Locale>;

        enum class Type
        {
            English,
            Russian,
        };

        enum class EndReason
        {
            None,
            JoiningOther,
            Timeout,
            EverybodyLeft,
            Kicked,
            Moved,
        };

    public:
        /// @brief Get locale type
        /// @return Locale type
        virtual inline Type type() = 0;

        /// @brief Get locale name
        /// @return Locale name
        virtual inline const char* name() = 0;

        /// @brief Get long locale name
        /// @return Long locale name
        virtual inline const char* longName() = 0;
    };
}

} // namespace kc
