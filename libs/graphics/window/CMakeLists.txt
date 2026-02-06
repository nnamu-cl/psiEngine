cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

set(NAME window)
project(${NAME})
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_library(SDL_LIBRARY NAMES SDL3 HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
find_library(SLANG_LIBRARY NAMES slang HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)

# WindowLib
file(GLOB_RECURSE WINDOWLIBFILES CONFIGURE_DEPENDS "windowlib/*.cpp" "windowlib/*.h")
add_library(WindowLib ${WINDOWLIBFILES})
set(IMGUI_DIR ../../../external/imgui) #adding the imgui lib
target_include_directories(WindowLib PUBLIC
        $ENV{VULKAN_SDK}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/windowlib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../core/include
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
target_link_libraries(WindowLib
    PRIVATE ${SDL_LIBRARY} ${SLANG_LIBRARY} CoreLib
    PUBLIC generator
)

# OpenWindow example
file(GLOB OPENWINDOWFILES "examples/OpenWindow/*.cpp" "examples/OpenWindow/*.h")
add_executable(OpenWindow ${OPENWINDOWFILES})
target_link_libraries(OpenWindow PRIVATE WindowLib)

# Copy LYGIA shader includes into build output so the runtime shader
# compiler can resolve #include "lygia/..." with assets/ as the root.
set(LYGIA_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/lygia)
add_custom_command(
        TARGET OpenWindow POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${LYGIA_SRC} $<TARGET_FILE_DIR:OpenWindow>/assets/lygia
        COMMENT "Copying LYGIA shaders to build directory"
)

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

# DefaultGameWorld CPU tests (no Vulkan required)
add_executable(tests_dgw tests/test_DefaultGameWorld.cpp)
target_link_libraries(tests_dgw PRIVATE WindowLib Catch2::Catch2WithMain)

# DefaultGameWorld Vulkan setup tests (requires GPU)
add_executable(tests_dgw_vulkan tests/test_DefaultGameWorld_VulkanSetup.cpp)
target_link_libraries(tests_dgw_vulkan PRIVATE WindowLib Catch2::Catch2WithMain)

# ShadSkin tests (no Vulkan required, just ImGui)
add_executable(tests_shadskin tests/test_ShadSkin.cpp)
target_link_libraries(tests_shadskin PRIVATE WindowLib Catch2::Catch2WithMain)

# Mesh generation tests (no Vulkan required, tests generator library integration)
add_executable(tests_meshgen tests/test_MeshGeneration.cpp)
target_link_libraries(tests_meshgen PRIVATE WindowLib Catch2::Catch2WithMain)

# Copy assets folder (shaders, fonts, etc.) to build directory
# Preserve directory structure: libs/graphics/window/assets/
set(ASSETS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/assets)
add_custom_command(
        TARGET tests_dgw_vulkan POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SRC} $<TARGET_FILE_DIR:tests_dgw_vulkan>/libs/graphics/window/assets
        COMMENT "Copying assets to test executable directory"
)

# Also copy assets for OpenWindow example
add_custom_command(
        TARGET OpenWindow POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SRC} $<TARGET_FILE_DIR:OpenWindow>/libs/graphics/window/assets
        COMMENT "Copying assets to OpenWindow executable directory"
)
