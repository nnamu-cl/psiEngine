cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(NAME tools)
project(${NAME})
set(CMAKE_CXX_STANDARD 20)

# ToolsLib - header-only library
# Since CircularBuffer is header-only, we create an INTERFACE library
add_library(ToolsLib INTERFACE)

target_include_directories(ToolsLib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Tests
Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
)

FetchContent_MakeAvailable(Catch2)

# CircularBuffer tests
add_executable(tests_circularbuffer tests/test_CircularBuffer.cpp)
target_link_libraries(tests_circularbuffer PRIVATE ToolsLib Catch2::Catch2WithMain)
