### --- Initial configuration --- ###
cmake_minimum_required(VERSION 3.10)
project(KontraBot VERSION 1.9)

### --- C++ standart configuration --- ###
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### --- Executable configuration --- ###
add_executable(KontraBot
    "source/common/curl.cpp"
    "source/common/js_interpreter.cpp"
    "source/youtube/error.cpp"
    "source/main.cpp"
)
target_include_directories(KontraBot PRIVATE "include/")

### --- Packages configuration --- ###
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(dpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(PkgConfig)
pkg_check_modules(MUJS REQUIRED IMPORTED_TARGET mujs)

### --- Linking configuration --- ###
target_include_directories(KontraBot PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(KontraBot PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(KontraBot PRIVATE
    fmt::fmt
    nlohmann_json::nlohmann_json
    CURL::libcurl
    ${FFMPEG_LIBRARIES}
    dpp::dpp
    spdlog::spdlog
    PkgConfig::MUJS
)
