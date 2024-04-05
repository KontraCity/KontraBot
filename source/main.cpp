// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/curl.hpp"
using namespace kc;

int main()
{
    try
    {
        Curl::Response response = Curl::Get("https://www.google.com/");
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
    catch (const std::invalid_argument& error)
    {
        std::cout << fmt::format("Connection error: {}\n", error.what());
    }
}
