cmake_minimum_required(VERSION 3.16)

project(NetSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(netsim
    src/package.cpp
    src/storage_types.cpp
    src/nodes.cpp
)
target_include_directories(netsim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---- GoogleTest ----
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(netsim_tests
    test/tests.cpp
)
target_link_libraries(netsim_tests PRIVATE netsim GTest::gtest_main)
target_include_directories(netsim_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(netsim_tests)