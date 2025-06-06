if(WIN32 AND NOT DEFINED CMAKE_GENERATOR)
  set(CMAKE_GENERATOR "Visual Studio 17 2022" CACHE STRING "Generator" FORCE)
endif()

cmake_minimum_required(VERSION 3.14)
project(CPUEmulator6502)

# create library
add_library(m6502_lib "lib/src/m6502.cpp")
target_include_directories(m6502_lib PUBLIC lib/src)

# main executable
add_executable(main_6502 "lib/src/main_6502.cpp")
target_include_directories(main_6502 PRIVATE lib/src)

# link library to exe
target_link_libraries(main_6502 PRIVATE m6502_lib)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# enable testing
enable_testing()

# test executable
set(TEST_SOURCES "test/src/cpuTests.cpp"
                 "test/src/loadRegisterTests.cpp"
                 "test/src/storeRegisterTests.cpp"
                 "test/src/jumpsAndCallsTests.cpp"
                 "test/src/stackOperationTests.cpp"
                 "test/src/logicalTests.cpp"
                 "test/src/registerTransferTests.cpp"
                 "test/src/incrementDecrementTests.cpp"
                 "test/src/branchTests.cpp"
                 "test/src/statusFlagChangeTests.cpp"
                 "test/src/arithmeticTests.cpp")

add_executable(m6502_tests ${TEST_SOURCES})

# link library to exe
target_link_libraries(m6502_tests PRIVATE m6502_lib GTest::gtest GTest::gtest_main)
target_include_directories(m6502_tests PRIVATE ${gtest_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(m6502_tests)