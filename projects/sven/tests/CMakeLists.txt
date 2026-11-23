cmake_minimum_required(VERSION 3.24)
project(bluetooth_audio_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(BUILD_GMOCK OFF CACHE BOOL "Build Google Mock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Install Google Test" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "Use the MSVC shared runtime" FORCE)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(bluetooth_audio_tests
    test/test_firmware/test_main.cpp
    test/test_firmware/test_demo_engine.cpp
    test/test_firmware/test_ultrasonic.cpp
    test/test_firmware/test_theremin.cpp
    test/test_firmware/test_hand_distance_filter.cpp
)
target_include_directories(bluetooth_audio_tests PRIVATE test/support ../include)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(bluetooth_audio_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()
target_link_libraries(bluetooth_audio_tests PRIVATE GTest::gtest)
include(GoogleTest)
gtest_discover_tests(bluetooth_audio_tests NO_PRETTY_VALUES)
