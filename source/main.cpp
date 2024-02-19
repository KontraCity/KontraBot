// STL modules
#include <iostream>

// Custom modules
#include "bot/locale/locale.hpp"
#include "bot/locale/locale_en.hpp"
#include "bot/locale/locale_ru.hpp"
using namespace kc;

static void Test(const Bot::Locale::Pointer& locale)
{
    std::cout << locale->SeekingTo("TIMESTAMP") << '\n';
}

int main()
{
    Test(std::make_unique<Bot::LocaleEn>());
    Test(std::make_unique<Bot::LocaleRu>());
}
