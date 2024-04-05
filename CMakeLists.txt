### --- Initial configuration --- ###
cmake_minimum_required(VERSION 3.10)
project(KontraBot VERSION 0)

### --- C++ configuration --- ###
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

### --- Executable configuration --- ###
add_executable(KontraBot "source/main.cpp"
    # Common modules
    "source/common/curl.cpp"

    # YouTube modules
    "source/youtube/error.cpp"
    "source/youtube/utility.cpp"
)
target_include_directories(KontraBot PRIVATE "include/")

### --- Linking configuration --- ###
find_package(fmt CONFIG REQUIRED)
find_package(CURL REQUIRED)
target_link_libraries(KontraBot PRIVATE
    fmt::fmt
    CURL::libcurl
)
