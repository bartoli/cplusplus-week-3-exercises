cmake_minimum_required(VERSION 3.14)
project(cplusplus_week3 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Keep CRT settings consistent on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(main src/main.cpp)
target_include_directories(main PUBLIC src)

enable_testing()
include(GoogleTest)

add_executable(test_main tests/main_test.cpp)
target_link_libraries(test_main PRIVATE main GTest::gtest_main)
gtest_discover_tests(test_main)
