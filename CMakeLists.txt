# reference to the CMU Database Group

cmake_minimum_required(VERSION 3.10)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD 17)              # Compile as C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++17 support
SET(CMAKE_BUILD_TYPE "Debug")

project(Dawn)

# CTest
enable_testing()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# gmock gtest https://crascit.com/2015/07/25/cmake-gtest/
configure_file("${PROJECT_SOURCE_DIR}/build_support/gtest_CMakeLists.txt.in" googletest-download/CMakeLists.txt)
# execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
#         WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
# execute_process(COMMAND "${CMAKE_COMMAND}" --build .
#         WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # don't override our compiler/linker options when building gtest
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src" "${CMAKE_BINARY_DIR}/googletest-build")

# Compiler flags
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra -Werror -march=native -Wno-reorder")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra -march=native -Wno-reorder")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-attributes")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fPIC")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC")
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fPIC")

set(GCC_COVERAGE_LINK_FLAGS    "-fPIC")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

# Includes.
set(DAWN_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)
set(DAWN_TEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test/include)
set(DAWN_THIRD_PARTY_INCLUDE_DIR 
    ${PROJECT_SOURCE_DIR}/third_party
    ${GTEST_INCLUDE_DIR}
)
include_directories(${DAWN_SRC_INCLUDE_DIR} ${DAWN_TEST_INCLUDE_DIR} ${DAWN_THIRD_PARTY_INCLUDE_DIR})
include_directories(BEFORE src) # This is needed for gtest.

add_subdirectory(src)
add_subdirectory(test)
