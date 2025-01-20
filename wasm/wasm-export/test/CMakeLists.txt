cmake_minimum_required(VERSION 3.27)
project(regex_fa_test)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set(TEST_LIB_NAME "wasm_export")
add_definitions(-DREGEX_FA_LOGGER)
add_subdirectory(../ ${TEST_LIB_NAME})

#-----------------------------------------------------------------------------------------------------------------------
#copied from https://github.com/google/googletest/tree/main/googletest
include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
#-----------------------------------------------------------------------------------------------------------------------

enable_testing()

add_definitions(-DGTEST)

#-----------------------------------------------------------------------------------------------------------------------

include_directories(sources)

file(GLOB TEST_SOURCES "sources/*.cpp")
foreach (ONE_TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(ONE_TEST_NAME "${ONE_TEST_SOURCE}" NAME)
    string(REPLACE ".cpp" "" ONE_TEST_NAME ${ONE_TEST_NAME})
    set(ONE_TEST_FULL_NAME "${PROJECT_NAME}-${ONE_TEST_NAME}")

    add_executable(${ONE_TEST_FULL_NAME} ${ONE_TEST_SOURCE})
    target_link_libraries(${ONE_TEST_FULL_NAME} gtest_main ${TEST_LIB_NAME})
    add_test(NAME ${ONE_TEST_NAME} COMMAND ${ONE_TEST_FULL_NAME})
endforeach ()
#-----------------------------------------------------------------------------------------------------------------------