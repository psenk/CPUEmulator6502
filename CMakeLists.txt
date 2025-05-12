cmake_minimum_required(VERSION 3.14)
project(CPUEmulator6502)

# main executable
add_executable(main_6502 "lib/src/main_6502.cpp")
target_include_directories(main_6502 PRIVATE lib/src)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SOURCES
    "test/src/6502_tests.cpp")

add_executable(main_6502_test ${TEST_SOURCES})

target_include_directories(main_6502_test PRIVATE lib/src)

target_link_libraries(main_6502_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(main_6502_test)