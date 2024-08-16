cmake_minimum_required(VERSION 3.27)

project(wasm_export)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set(TARGET_NAME ${PROJECT_NAME})

add_library(${TARGET_NAME} INTERFACE)

file(GLOB LIB_SOURCES "include/wasm-export/*.hpp")
target_sources(${TARGET_NAME} INTERFACE ${LIB_SOURCES})


add_subdirectory(../../ regex_fa)
target_link_libraries(${TARGET_NAME} INTERFACE regex_fa)


include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(${TARGET_NAME} INTERFACE nlohmann_json::nlohmann_json)

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
