// STL modules
#include <iostream>

// Custom modules
#include "youtube/error.hpp"
using namespace kc;

int main()
{
    try
    {
        throw Youtube::YoutubeError(
            Youtube::YoutubeError::Type::Unplayable,
            "sJL6WA-aGkQ",
            "Video unavailable",
            "The uploader has not made this video available in your country"
        );
    }
    catch (const Youtube::YoutubeError& error)
    {
        std::cerr << "YouTube error occured: " << error.what() << '\n';
    }

    try
    {
        throw Youtube::LocalError(
            Youtube::LocalError::Type::ShortsPlaylist,
            "PLnN2bBxGARv7fRxsCcWaxvGE6sn5Ypp1H"
        );
    }
    catch (const Youtube::LocalError& error)
    {
        std::cerr << "Local error occured: " << error.what() << '\n';
    }
}
