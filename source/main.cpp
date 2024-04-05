// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "youtube/client.hpp"
using namespace kc;

int main()
{
    try
    {
        Curl::Response response = Youtube::Client::Instance->requestApi(
            Youtube::Client::Type::Web,
            "player",
            { {"videoId", "WVOH00wVFbc"} }
        );

        std::cout << fmt::format(
            "Response code: {}, response length: {}\n",
            response.code,
            response.data.length()
        );
    }
    catch (const std::runtime_error& error)
    {
        std::cout << fmt::format("Internal error: {}\n", error.what());
    }
}
