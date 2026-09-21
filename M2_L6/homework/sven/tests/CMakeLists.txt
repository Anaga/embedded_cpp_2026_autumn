cmake_minimum_required(VERSION 3.24)
project(firmware_google_tests LANGUAGES CXX)

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
# test_main.cpp includes the real firmware source exactly once.
add_executable(firmware_tests test/test_firmware/test_main.cpp)
target_include_directories(firmware_tests PRIVATE test/support)
# The shared test source supplies main() for both CMake and PlatformIO.
target_link_libraries(firmware_tests PRIVATE GTest::gtest)

include(GoogleTest)
gtest_discover_tests(firmware_tests NO_PRETTY_VALUES)
