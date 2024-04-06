#pragma once

// Custom modules
#include "bot/locale/locale.hpp"

namespace kc {

namespace Bot
{
    class LocaleRu : public Locale
    {
    public:
        static constexpr Locale::Type Type = Locale::Type::Russian;
        static constexpr const char* Name = "ru";
        static constexpr const char* LongName = u8"Русский";

    public:
        /// @brief Get number's cardinal ending
        /// @param number The number in question
        /// @return Number's cardinal ending
        static inline const char* Cardinal(size_t number)
        {
            if (number == 1)
                return "";
            if (number >= 2 && number <= 4)
                return u8"а";
            return u8"ов";
        }

    public:
        /// @brief Get locale type
        /// @return Locale type
        virtual inline Locale::Type type()
        {
            return Type;
        }

        /// @brief Get locale name
        /// @return Locale name
        virtual inline const char* name()
        {
            return Name;
        }

        /// @brief Get long locale name
        /// @return Long locale name
        virtual inline const char* longName()
        {
            return LongName;
        }
    };
}

} // namespace kc
