# reference to the CMU Database Group

# set(CFiles ${PROJECT_SOURCE_DIR}/src/sql/lex.yy.c ${PROJECT_SOURCE_DIR}/src/sql/y.tab.c )

file(GLOB_RECURSE dawn_sources ${PROJECT_SOURCE_DIR}/src/*/*.cpp ${PROJECT_SOURCE_DIR}/src/*/*/*.cpp ${PROJECT_SOURCE_DIR}/src/*/*/*/*.cpp)
add_library(dawn_shared SHARED ${dawn_sources})

######################################################################################################################
# THIRD-PARTY SOURCES
######################################################################################################################

# murmur3
file(GLOB_RECURSE murmur3_sources
        ${PROJECT_SOURCE_DIR}/third_party/murmur3/*.cpp ${PROJECT_SOURCE_DIR}/third_party/murmur3/*.h)
add_library(thirdparty_murmur3 SHARED ${murmur3_sources})
target_link_libraries(dawn_shared thirdparty_murmur3)