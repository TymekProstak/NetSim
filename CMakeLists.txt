cmake_minimum_required(VERSION 3.16)
project(NetSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Library with project code ---
add_library(netsim
    src/helpers.cpp
    src/nodes.cpp
    src/package.cpp
    src/storage_types.cpp
    src/factory.cpp
    src/io.cpp
)
target_include_directories(netsim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(Aplikacja src/main.cpp)
target_link_libraries(Aplikacja PRIVATE netsim)

add_executable(LoadFactory test/io_test.cpp)
target_link_libraries(LoadFactory PRIVATE netsim)

add_executable(SimulationApp test/simulation_test.cpp)
target_link_libraries(SimulationApp PRIVATE netsim)

add_custom_command(TARGET LoadFactory POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/test/load_factory.txt"
    "$<TARGET_FILE_DIR:LoadFactory>/load_factory.txt"
)

add_custom_command(TARGET SimulationApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/test/load_factory.txt"
    "$<TARGET_FILE_DIR:SimulationApp>/load_factory.txt"
)

option(ENABLE_TESTS "Build unit tests" ON)
if(ENABLE_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(Testy
        test/tests.cpp

    )
    target_link_libraries(Testy PRIVATE netsim gtest gtest_main)

    include(GoogleTest)
    gtest_discover_tests(Testy)
endif()