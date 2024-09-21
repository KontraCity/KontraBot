## --- Project configuration --- ##
cmake_minimum_required(VERSION 3.10)
project(KontraBot)

## --- Compiler configuration --- ##
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
include_directories("include/")

## --- KontraBot library configuration --- ##
add_library(kontrabotbase
    # Bot handlers
    "source/bot/handlers/on_autocomplete.cpp"
    "source/bot/handlers/on_button_click.cpp"
    "source/bot/handlers/on_log.cpp"
    "source/bot/handlers/on_message_create.cpp"
    "source/bot/handlers/on_ready.cpp"
    "source/bot/handlers/on_select_click.cpp"
    "source/bot/handlers/on_slashcommand.cpp"
    "source/bot/handlers/on_voice_ready.cpp"
    "source/bot/handlers/on_voice_server_update.cpp"
    "source/bot/handlers/on_voice_state_update.cpp"
    "source/bot/handlers/on_voice_track_marker.cpp"

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
    "source/common/cache.cpp"
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
if (WIN32)
    ## --- Packages configuration --- ##
    find_package(fmt CONFIG REQUIRED)
    find_package(spdlog CONFIG REQUIRED)
    find_package(CURL REQUIRED)
    find_package(dpp CONFIG REQUIRED)
    find_package(Boost REQUIRED)
    find_package(FFMPEG REQUIRED)

    ## --- PkgConfig packages configuration --- ##
    find_package(PkgConfig)
    pkg_check_modules(MUJS REQUIRED IMPORTED_TARGET mujs)

    ## --- KontraBot library dependencies configuration --- ##
    target_include_directories(kontrabotbase PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(kontrabotbase PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(kontrabotbase PRIVATE
        fmt::fmt
        spdlog::spdlog
        CURL::libcurl
        dpp::dpp
        Boost::boost
        "bcrypt"
        ${FFMPEG_LIBRARIES}
        PkgConfig::MUJS
    )
elseif (UNIX)
    ## --- KontraBot library dependencies configuration --- ##
    target_link_libraries(kontrabotbase PRIVATE
        "fmt"
        "spdlog"
        "curl"
        "dpp"
        "boost_regex"
        "avcodec" "avformat" "avutil" "swresample"
        "mujs"
    )
endif()

## --- KontraBot executable configuration --- ##
add_executable(KontraBot "source/main.cpp")
target_link_libraries(KontraBot PRIVATE "kontrabotbase")
if (WIN32)
    target_link_libraries(KontraBot PRIVATE fmt::fmt spdlog::spdlog)
endif()

## --- Testing configuration --- ##
option(BUILD_TESTS "Build KontraBot tests" YES)
if (BUILD_TESTS)
    add_subdirectory("tests/")
endif()
