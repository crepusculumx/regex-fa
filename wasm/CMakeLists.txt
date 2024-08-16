cmake_minimum_required(VERSION 3.27)
project(fa_wasm)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(wasm-export)

add_executable(fa_wasm main.cpp)

add_definitions(-DREGEX_FA_LOGGER)

#-----------------------------------------------------------------------------------------------------------------------
##https://json.nlohmann.me/integration/cmake/#fetchcontent
#include(FetchContent)
#FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
#FetchContent_MakeAvailable(json)
#

#-----------------------------------------------------------------------------------------------------------------------

target_link_libraries(fa_wasm wasm_export nlohmann_json::nlohmann_json)
set_target_properties(fa_wasm PROPERTIES LINK_FLAGS "-sEXPORTED_FUNCTIONS=_LibCall -sEXPORTED_RUNTIME_METHODS=ccall,cwrap")
