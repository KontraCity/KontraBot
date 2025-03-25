cmake_minimum_required(VERSION 3.20)
project(KontraBot)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
include_directories("include/")

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(dpp CONFIG REQUIRED)
find_package(Boost REQUIRED)

set(BuildExample NO)
add_subdirectory("ytcpp/")
include_directories("ytcpp/ytcpp/include/")

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

    "source/core/config.cpp"
    "source/core/downloader.cpp"
    "source/core/io.cpp"
    "source/core/utility.cpp"
)
target_link_libraries(KontraBot PRIVATE
    fmt::fmt
    spdlog::spdlog
    dpp::dpp
    Boost::boost
    ytcpp
)

if (WIN32)
    find_package(FFMPEG REQUIRED)
    target_include_directories(KontraBot PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(KontraBot PRIVATE ${FFMPEG_LIBRARY_DIRS})
endif()
target_link_libraries(KontraBot PRIVATE "avcodec" "avformat" "avutil" "swresample")
