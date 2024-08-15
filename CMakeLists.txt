## --- Project configuration --- ##
cmake_minimum_required(VERSION 3.10)
project(KontraBot VERSION 0)

## --- Compiler configuration --- ##
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
include_directories("include/")

## --- KontraBot library configuration --- ##
add_library(kontrabot
    # Bot modules
    "source/bot/commands.cpp"
    "source/bot/bot.cpp"
    "source/bot/info.cpp"
    "source/bot/locale.cpp"
    "source/bot/player.cpp"
    "source/bot/signal.cpp"
    "source/bot/timeout.cpp"
    "source/bot/types.cpp"

    # Common modules
    "source/common/config.cpp"
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
target_link_libraries(kontrabot PRIVATE
    "curl"
    "fmt"
    "spdlog"
    "dpp"
    "mujs"
    "boost_regex"
    "avcodec" "avformat" "avutil" "swresample"
)

## --- KontraBot executable configuration --- ##
add_executable(KontraBot "source/main.cpp")
target_link_libraries(KontraBot PRIVATE "kontrabot")

## --- Testing configuration --- ##
option(BUILD_TESTS "Build KontraBot tests" YES)
if (BUILD_TESTS)
    add_subdirectory("tests/")
endif()
