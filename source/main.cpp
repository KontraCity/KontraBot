// STL modules
#include <iostream>

// Custom modules
#include "common/curl.hpp"
using namespace kc;

int main()
{
    try
    {
        Curl::Response response = Curl::Get("https://www.youtube.com");
        std::cout << "https://www.youtube.com access: " << response.code << '\n';
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Runtime error: " << error.what() << '\n';
    }
    catch (const std::invalid_argument& error)
    {
        std::cerr << "Invalid argument: " << error.what() << '\n';
    }
}
