cmake_minimum_required(VERSION 3.10)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD 17)              # Compile as C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++17 support

project(Dawn)

# CTest
enable_testing()

# gmock gtest https://crascit.com/2015/07/25/cmake-gtest/
configure_file("${PROJECT_SOURCE_DIR}/build_support/gtest_CMakeLists.txt.in" googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # don't override our compiler/linker options when building gtest
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src" "${CMAKE_BINARY_DIR}/googletest-build")

# Compiler flags

# Includes.
set(DAWN_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)
set(DAWN_TEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test/include)
set(DAWN_THIRD_PARTY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party)
include_directories(${DAWN_SRC_INCLUDE_DIR} ${DAWN_TEST_INCLUDE_DIR} ${DAWN_THIRD_PARTY_INCLUDE_DIR})
include_directories(BEFORE src) # This is needed for gtest.

add_subdirectory(src)
add_subdirectory(test)