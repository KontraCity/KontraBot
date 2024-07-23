## --- Project configuration --- ##
cmake_minimum_required(VERSION 3.10)
project(KontraBot VERSION 0)

## --- Compiler configuration --- ##
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
include_directories("include/")

## --- HTTP/3 and QUIC configuration --- ##
option(ENABLE_HTTP_3 "Enable HTTP/3 and QUIC support" ON)
if (ENABLE_HTTP_3)
    set(LIB_DIRECTORY "/usr/local/lib")
    set(LIB_64_DIRECTORY "/usr/local/lib64")
    set(NETWORKING_LIBRARIES
        "${LIB_DIRECTORY}/libcurl.a"
        "${LIB_DIRECTORY}/libnghttp3.a"
        "${LIB_DIRECTORY}/libngtcp2.a"
        "${LIB_DIRECTORY}/libngtcp2_crypto_quictls.a"
        "${LIB_64_DIRECTORY}/libssl.a"
        "${LIB_64_DIRECTORY}/libcrypto.a"
        "z"
    )
    add_compile_definitions(HTTP3_ENABLED)
    message("HTTP/3 and QUIC support is ENABLED")
else()
    set(NETWORKING_LIBRARIES "curl")
    message("HTTP/3 and QUIC support is DISABLED")
endif()

## --- KontraBot library configuration --- ##
add_library(kontrabot
    # Bot modules
    "source/bot/commands.cpp"
    "source/bot/bot.cpp"
    "source/bot/config.cpp"
    "source/bot/info.cpp"
    "source/bot/locale.cpp"
    "source/bot/player.cpp"
    "source/bot/signal.cpp"
    "source/bot/timeout.cpp"
    "source/bot/types.cpp"

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
target_link_libraries(kontrabot PRIVATE
    "fmt"
    "spdlog"
    "dpp"
    "mujs"
    "boost_regex"
    "avcodec" "avformat" "avutil" "swresample"
    ${NETWORKING_LIBRARIES}
)

## --- KontraBot executable configuration --- ##
add_executable(KontraBot "source/main.cpp")
target_link_libraries(KontraBot PRIVATE "kontrabot")

## --- Testing configuration --- ##
option(BUILD_TESTS "Build KontraBot tests" ON)
if (BUILD_TESTS)
    add_subdirectory("tests/")
endif()
