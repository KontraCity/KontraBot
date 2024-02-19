// STL modules
#include <iostream>

// Custom modules
#include "bot/config.hpp"
using namespace kc;

int main()
{
    Bot::Config::Pointer config = std::make_shared<Bot::Config>();
    std::cout << config->discordBotApiToken() << '\n';
}
