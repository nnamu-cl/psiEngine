cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

set(NAME window)
project(${NAME})
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_library(SDL_LIBRARY NAMES SDL3 HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)

# WindowLib
file(GLOB WINDOWLIBFILES "windowlib/*.cpp" "windowlib/*.h")
add_library(WindowLib ${WINDOWLIBFILES})
set(IMGUI_DIR ../../../external/imgui) #adding the imgui lib
target_include_directories(WindowLib PUBLIC
        $ENV{VULKAN_SDK}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/windowlib
        ${IMGUI_DIR} ${IMGUI_DIR}/backends
)

target_sources(WindowLib PRIVATE
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
)

target_compile_definitions(WindowLib PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK "IMGUI_IMPL_VULKAN_VOLK_FILENAME=<volk/volk.h>")
target_link_libraries(WindowLib PRIVATE ${SDL_LIBRARY})

# OpenWindow example
file(GLOB OPENWINDOWFILES "examples/OpenWindow/*.cpp" "examples/OpenWindow/*.h")
add_executable(OpenWindow ${OPENWINDOWFILES})
target_link_libraries(OpenWindow PRIVATE WindowLib)

# tests
Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)
add_executable(tests tests/test_WindowTests.cpp)
target_link_libraries(tests PRIVATE WindowLib Catch2::Catch2WithMain)
