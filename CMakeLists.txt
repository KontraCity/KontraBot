﻿## --- Project configuration --- ##
cmake_minimum_required(VERSION 3.10)
project(KontraBot VERSION 0)

## --- Compiler configuration --- ##
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
include_directories("include/")

## --- DPI workaround configuration --- ##
option(ENABLE_DPI_WORKAROUND "Enable DPI workaround" YES)
if (ENABLE_DPI_WORKAROUND)
    add_compile_definitions(DPI_WORKAROUND_ENABLED)
    message("[CONFIG] DPI workaround is ENABLED")
else()
    message("[CONFIG] DPI workaround is DISABLED")
endif()

## --- HTTP/3 and QUIC configuration --- ##
option(ENABLE_HTTP_3 "Enable HTTP/3 and QUIC support" YES)
if (ENABLE_HTTP_3)
    set(CURL_LIBS_DIRECTORY "/usr/local/lib" CACHE STRING "Directory where custom built Curl and other libraries are installed")
    set(OPENSSL_LIBS_DIRECTORY "/usr/local/lib64" CACHE STRING "Directory where custom built OpenSSL libraries are installed")
    set(NETWORKING_LIBRARIES
        "${CURL_LIBS_DIRECTORY}/libcurl.a"
        "${CURL_LIBS_DIRECTORY}/libnghttp3.a"
        "${CURL_LIBS_DIRECTORY}/libngtcp2.a"
        "${CURL_LIBS_DIRECTORY}/libngtcp2_crypto_quictls.a"
        "${OPENSSL_LIBS_DIRECTORY}/libssl.a"
        "${OPENSSL_LIBS_DIRECTORY}/libcrypto.a"
        "z"
    )
    add_compile_definitions(HTTP3_ENABLED)
    message("[CONFIG] HTTP/3 and QUIC support is ENABLED")
    message("[CONFIG] Curl libraries directory: \"${CURL_LIBS_DIRECTORY}\"")
    message("[CONFIG] OpenSSL libraries directory: \"${OPENSSL_LIBS_DIRECTORY}\"")
else()
    set(NETWORKING_LIBRARIES "curl")
    message("[CONFIG] HTTP/3 and QUIC support is DISABLED")
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
option(BUILD_TESTS "Build KontraBot tests" YES)
if (BUILD_TESTS)
    add_subdirectory("tests/")
endif()
