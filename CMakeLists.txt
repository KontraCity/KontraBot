### --- Initial configuration --- ###
cmake_minimum_required(VERSION 3.10)
project(KontraBot VERSION 0)

### --- C++ configuration --- ###
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

### --- Executable configuration --- ###
add_executable(KontraBot "source/main.cpp"
    # Bot modules
    "source/bot/info/info.cpp"
    "source/bot/info/types.cpp"
    "source/bot/bot.cpp"
    "source/bot/config.cpp"
    "source/bot/locale.cpp"
    "source/bot/player.cpp"
    "source/bot/signal.cpp"
    "source/bot/timeout.cpp"

    # Common modules
    "source/common/curl.cpp"
    "source/common/interpreter.cpp"
    "source/common/stopwatch.cpp"
    "source/common/utility.cpp"

    # YouTube modules
    "source/youtube/client.cpp"
    "source/youtube/error.cpp"
    "source/youtube/extractor.cpp"
    "source/youtube/playlist.cpp"
    "source/youtube/search.cpp"
    "source/youtube/utility.cpp"
    "source/youtube/video.cpp"
)
target_include_directories(KontraBot PRIVATE "include/")

### --- Linking configuration --- ###
target_link_libraries(KontraBot PRIVATE
    "fmt"
    "curl"
    "dpp"
    "spdlog"
    "mujs"
    "boost_regex"
    "avcodec" "avformat" "avutil"
)
