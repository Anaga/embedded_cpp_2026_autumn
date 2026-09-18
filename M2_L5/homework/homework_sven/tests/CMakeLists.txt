cmake_minimum_required(VERSION 3.24)
project(temperature_google_tests LANGUAGES CXX)

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
# test_main.cpp includes the real homework source exactly once.
add_executable(temperature_tests test/test_temperature/test_main.cpp)
target_include_directories(temperature_tests PRIVATE test/support)
# The shared test source supplies main() for both CMake and PlatformIO.
target_link_libraries(temperature_tests PRIVATE GTest::gtest)

include(GoogleTest)
gtest_discover_tests(temperature_tests NO_PRETTY_VALUES)
