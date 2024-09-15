## --- Compiler configuration --- ##
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
include_directories("include/")

## --- Test executable configuration --- ##
add_executable(Test
    "source/main.cpp"
    "source/video.cpp"
)
target_link_libraries(Test PRIVATE "kontrabotbase")
if (WIN32)
    target_link_libraries(Test PRIVATE fmt::fmt spdlog::spdlog)
endif()

## --- Testing configuration --- ##
enable_testing()
add_test(NAME "Video" COMMAND "Test" "--video")
