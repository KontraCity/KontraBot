cmake_minimum_required(VERSION 3.20)
project(KontraBot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
include_directories("include/")

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(dpp CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(PkgConfig)
pkg_check_modules(MUJS REQUIRED IMPORTED_TARGET mujs)

add_executable(KontraBot "source/main.cpp"
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

    "source/bot/commands.cpp"
    "source/bot/bot.cpp"
    "source/bot/info.cpp"
    "source/bot/locale.cpp"
    "source/bot/player.cpp"
    "source/bot/signal.cpp"
    "source/bot/timeout.cpp"
    "source/bot/types.cpp"

    "source/core/cache.cpp"
    "source/core/config.cpp"
    "source/core/curl.cpp"
    "source/core/interpreter.cpp"
    "source/core/utility.cpp"

    "source/youtube/client.cpp"
    "source/youtube/error.cpp"
    "source/youtube/extractor.cpp"
    "source/youtube/playlist.cpp"
    "source/youtube/search.cpp"
    "source/youtube/utility.cpp"
    "source/youtube/video.cpp"
)
target_link_libraries(KontraBot PRIVATE
    fmt::fmt
    spdlog::spdlog
    CURL::libcurl
    dpp::dpp
    Boost::boost
    PkgConfig::MUJS
)

if (WIN32)
    find_package(FFMPEG REQUIRED)
    target_include_directories(KontraBot PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(KontraBot PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(KontraBot PRIVATE "bcrypt" ${FFMPEG_LIBRARIES})
elseif (UNIX)
    target_link_libraries(KontraBot PRIVATE "avcodec" "avformat" "avutil" "swresample")
endif()
