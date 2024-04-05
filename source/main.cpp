// STL modules
#include <iostream>

// Library {fmt}
#include <fmt/format.h>

// Custom modules
#include "common/utility.hpp"
#include "youtube/extractor.hpp"
using namespace kc;

int main()
{
    try
    {
        Youtube::Extractor extractor("9OFpfTd0EIs");
        while (true)
        {
            Youtube::Extractor::Frame frame = extractor.extractFrame();
            if (frame.empty())
                break;

            fmt::print(
                "{} {}\n",
                Utility::NiceString(pt::time_duration(0, 0, 0, frame.timestamp() * 1'000)),
                frame.size()
            );
        }
    } 
    catch (const Youtube::YoutubeError& error)
    {
        std::cout << fmt::format("YouTube error: {}\n", error.what());
    }
    catch (const Youtube::LocalError& error)
    {
        std::cout << fmt::format("Local error: {}\n", error.what());
    }
}
