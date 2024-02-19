// STL modules
#include <iostream>

// Custom modules
#include "bot/timeout.hpp"
using namespace kc;

int main()
{
    Bot::Timeout timeout([]() { std::cout << "Timeout!\n"; }, 1);
    while (true)
    {
        char input;
        std::cin >> input;
        switch (input)
        {
            case 'E':
                timeout.enable();
                break;
            case 'D':
                timeout.disable();
                break;
            case 'R':
                timeout.reset();
                break;
        }
        std::cout << (timeout.enabled() ? "Enabled" : "Disabled") << '\n';
    }
}
