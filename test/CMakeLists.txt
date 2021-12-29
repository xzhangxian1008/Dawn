# reference to the CMU Database Group

file(GLOB DAWN_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*test.cpp")

# valgrind
find_program(VALGRIND_BIN valgrind)
if ("${VALGRIND_BIN}" STREQUAL "VALGRIND_BIN-NOTFOUND")
    message(WARNING "Dawn/test couldn't find valgrind.")
else()
    message(STATUS "Dawn/test found valgrind at ${VALGRIND_BIN}")
endif()

set(VALGRIND_OPTIONS
        --error-exitcode=1                      # if leaks are detected, return nonzero value
        # --gen-suppressions=all  # uncomment for leak suppression syntax
        --leak-check=full                       # detailed leak information
        --soname-synonyms=somalloc=*jemalloc*   # also intercept jemalloc
        --trace-children=yes                    # trace child processes
        --track-origins=yes                     # track origin of uninitialized values
        )
set(VALGRIND_SUPPRESSIONS_FILE "${PROJECT_SOURCE_DIR}/build_support/valgrind.supp")

# Copy test files to destination
file(GLOB_RECURSE test_files ${PROJECT_SOURCE_DIR}/test/sql/test_cases/*)
file(COPY ${test_files} DESTINATION ${CMAKE_BINARY_DIR}/test/)

##########################################
# "make XYZ_test"
##########################################
foreach (dawn_test_source ${DAWN_TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(dawn_test_filename ${dawn_test_source} NAME)
    string(REPLACE ".cpp" "" dawn_test_name ${dawn_test_filename})

    # Add the test target separately and as part of "make check-tests".
    add_executable(${dawn_test_name} EXCLUDE_FROM_ALL ${dawn_test_source})

    target_link_libraries(${dawn_test_name} dawn_shared gtest gmock_main)

    # Set test target properties and dependencies.
    set_target_properties(${dawn_test_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
        COMMAND ${dawn_test_name}
    )

    # Add the test under CTest.
    add_test(${dawn_test_name} ${VALGRIND_BIN} ${VALGRIND_OPTIONS}
    --suppressions=${VALGRIND_SUPPRESSIONS_FILE} ${CMAKE_BINARY_DIR}/test/${dawn_test_name}
    --gtest_color=yes --gtest_output=xml:${CMAKE_BINARY_DIR}/test/unit_${test_name}.xml)
    add_test(${dawn_test_name} ${CMAKE_BINARY_DIR}/test/${dawn_test_name} --gtest_color=yes
            --gtest_output=xml:${CMAKE_BINARY_DIR}/test/${dawn_test_name}.xml)
endforeach(dawn_test_source ${DAWN_TEST_SOURCES})
