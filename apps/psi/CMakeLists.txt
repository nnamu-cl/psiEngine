cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(NAME psiQuantum)
project(${NAME})
set(CMAKE_CXX_STANDARD 20)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Find required libraries
find_library(SDL_LIBRARY NAMES SDL3 HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
find_library(SLANG_LIBRARY NAMES slang HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)

# Gather all source files
file(GLOB_RECURSE PSI_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
)

file(GLOB_RECURSE PSI_HEADERS CONFIGURE_DEPENDS
    "src/*.h"
    "src/*.hpp"
)

# Create the executable
add_executable(${NAME} ${PSI_SOURCES} ${PSI_HEADERS})

# Include directories
target_include_directories(${NAME} PRIVATE
    $ENV{VULKAN_SDK}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/colors
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/graphics/window/windowlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/generator/include
)

# Link against WindowLib and other dependencies
target_link_libraries(${NAME} PRIVATE
    WindowLib
    generator
    ${SDL_LIBRARY}
    ${SLANG_LIBRARY}
)

# Compiler definitions
target_compile_definitions(${NAME} PRIVATE
    VK_NO_PROTOTYPES
    IMGUI_IMPL_VULKAN_USE_VOLK
    "IMGUI_IMPL_VULKAN_VOLK_FILENAME=<volk/volk.h>"
)

if(WIN32)
    target_compile_definitions(${NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Set debugger working directory to where the executable is (where assets are copied)
set_target_properties(${NAME} PROPERTIES
    DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${NAME}>
)

# Copy LYGIA shader includes into build output
set(LYGIA_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../external/lygia)
add_custom_command(
    TARGET ${NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${LYGIA_SRC} $<TARGET_FILE_DIR:${NAME}>/assets/lygia
    COMMENT "Copying LYGIA shaders to PsiApplication build directory"
)

# Copy window library assets (shaders, fonts, etc.)
set(WINDOW_ASSETS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/graphics/window/assets)
add_custom_command(
    TARGET ${NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${WINDOW_ASSETS_SRC} $<TARGET_FILE_DIR:${NAME}>/libs/graphics/window/assets
    COMMENT "Copying window library assets to PsiApplication build directory"
)

# Copy shaders to the location where DefaultGameWorld expects them
# DefaultGameWorld looks for: libs/graphics/window/shaders/...
set(WINDOW_SHADERS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/graphics/window/assets/shaders)
add_custom_command(
    TARGET ${NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${WINDOW_SHADERS_SRC} $<TARGET_FILE_DIR:${NAME}>/libs/graphics/window/shaders
    COMMENT "Copying shaders to expected location for DefaultGameWorld"
)

# Copy psi-specific assets if they exist (includes icons folder)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    add_custom_command(
        TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${NAME}>/apps/psi/assets
        COMMENT "Copying PSI application assets to build directory"
    )
endif()

# ============================================================================
# Tests
# ============================================================================
Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.8.1
)

FetchContent_MakeAvailable(Catch2)

# PSI Core tests (no Vulkan required - tests Transform, Scene, GameObject)
add_executable(psi_tests tests/test_PsiCore.cpp)
target_link_libraries(psi_tests PRIVATE WindowLib Catch2::Catch2WithMain)

# Include directories for tests
target_include_directories(psi_tests PRIVATE
    $ENV{VULKAN_SDK}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/colors
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/graphics/window/windowlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/generator/include
)

target_compile_definitions(psi_tests PRIVATE
    VK_NO_PROTOTYPES
    IMGUI_IMPL_VULKAN_USE_VOLK
    "IMGUI_IMPL_VULKAN_VOLK_FILENAME=<volk/volk.h>"
)

if(WIN32)
    target_compile_definitions(psi_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
