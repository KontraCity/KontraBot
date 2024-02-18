// STL modules
#include <iostream>

// Custom modules
#include "youtube/client.hpp"
using namespace kc;

int main()
{
    auto response = Youtube::Client::Instance->requestApi(Youtube::Client::Type::Web, "player", { {"videoId", "1V_xRb0x9aw"} });
    std::cout << response.code << '\n';
}
