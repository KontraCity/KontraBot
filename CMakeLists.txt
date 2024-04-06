### --- Initial configuration --- ###
cmake_minimum_required(VERSION 3.10)
project(KontraBot VERSION 0)

### --- C++ configuration --- ###
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

### --- Executable configuration --- ###
add_executable(KontraBot "source/main.cpp"
    # Bot modules
    "source/bot/config.cpp"
    "source/bot/locale.cpp"
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
 "source/bot/signal.cpp")
target_include_directories(KontraBot PRIVATE "include/")

### --- Linking configuration --- ###
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(dpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(PkgConfig)
pkg_check_modules(MUJS REQUIRED IMPORTED_TARGET mujs)
target_link_libraries(KontraBot PRIVATE
    fmt::fmt
    nlohmann_json::nlohmann_json
    CURL::libcurl
    dpp::dpp
    spdlog::spdlog
    PkgConfig::MUJS
)

### --- Platform dependent linking configuration --- ###
if (WIN32)
    find_package(FFMPEG REQUIRED)
    target_include_directories(KontraBot PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(KontraBot PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(KontraBot PRIVATE ${FFMPEG_LIBRARIES})
elseif (UNIX)
    target_link_libraries(KontraBot PRIVATE "avcodec" "avformat" "avutil")
endif()
