file(GLOB_RECURSE dawn_sources ${PROJECT_SOURCE_DIR}/src/*/*.cpp ${PROJECT_SOURCE_DIR}/src/*/*/*.cpp ${PROJECT_SOURCE_DIR}/src/*/*/*/*.cpp)
add_library(dawn_shared SHARED ${dawn_sources})