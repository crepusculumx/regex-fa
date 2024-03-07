cmake_minimum_required(VERSION 3.27)

project(regex_fa)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set(TARGET_NAME ${PROJECT_NAME})

add_library(${TARGET_NAME} INTERFACE)

file(GLOB LIB_SOURCES "include/regex-fa/*.hpp")
target_sources(${TARGET_NAME} INTERFACE ${LIB_SOURCES})

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
