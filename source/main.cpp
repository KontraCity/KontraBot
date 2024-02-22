// STL modules
#include <iostream>

// Custom modules
#include "bot/signal.hpp"
using namespace kc;

void Test(const std::string& signalString)
{
    Bot::Signal signal(signalString);
    std::cout << "Type: " << static_cast<int>(signal.type()) << '\n';
    std::cout << "Data: " << signal.data() << '\n';
}

int main()
{
    try
    {
        Bot::Signal signal(Bot::Signal::Type::Played, "1TO48Cnl66w");
        Test(signal);
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
