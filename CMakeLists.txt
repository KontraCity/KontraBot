### --- Initial configuration --- ###
cmake_minimum_required(VERSION 3.10)
project(KontraBot VERSION 0)

### --- C++ configuration --- ###
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### --- Executable configuration --- ###
add_executable(KontraBot "source/main.cpp")
target_include_directories(KontraBot PRIVATE "include/")

### --- Linking configuration --- ###
find_package(fmt CONFIG REQUIRED)
target_link_libraries(KontraBot PRIVATE fmt::fmt)
