// STL modules
#include <iostream>

// Custom modules
#include "bot/stats.hpp"
using namespace kc;

int main()
{
    try
    {
        Bot::Stats stats(0);
        std::cout << "Locale: " << stats.locale()->LocaleName() << '\n';
        std::cout << "Timeout duration: " << stats.timeoutDuration() << '\n';
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Runtime error: " << error.what() << '\n';
        return 1;
    }
    catch (const std::invalid_argument& error)
    {
        std::cerr << "Invalid argument: " << error.what() << '\n';
        return 1;
    }
}
