cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

set(NAME decision_engine)
project(${NAME})
set(CMAKE_CXX_STANDARD 23)


# We make use of csv-reader for reading csv files
add_subdirectory(../../external/csv-parser csv-build)
add_subdirectory(../../external/zpp_bits zpp_bits)

set(LUAJIT_DIR /home/namusanga/Documents/GitHub/LuaJIT)

# Define the files that should be included
file(GLOB_RECURSE decision_engine_files "./*.cpp" "./*.h")
list(FILTER decision_engine_files EXCLUDE REGEX ".*/cmake-build[^/]*/.*")

add_executable(${NAME} ${decision_engine_files})
target_compile_options(${NAME} PUBLIC -Wno-template-body -no-pie )
target_link_options(${NAME} PRIVATE -no-pie -Wl,-E)
target_compile_definitions(${NAME} PRIVATE IMGUI_DOCKING IMGUI_DISABLE_OBSOLETE_KEYIO)



# Compiler definitions
target_compile_definitions(${NAME} PRIVATE
        VK_NO_PROTOTYPES
        IMGUI_IMPL_VULKAN_USE_VOLK
        "IMGUI_IMPL_VULKAN_VOLK_FILENAME=<volk/volk.h>"
)


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

# Copy shaders to the location where DefaultGameWorld expects them
# DefaultGameWorld looks for: libs/graphics/window/shaders/...
set(WINDOW_SHADERS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/graphics/window/assets/shaders)
add_custom_command(
        TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${WINDOW_SHADERS_SRC} $<TARGET_FILE_DIR:${NAME}>/libs/graphics/window/shaders
        COMMENT "Copying shaders to expected location for DefaultGameWorld"
)


# Include directories
target_include_directories(${NAME} PUBLIC
        $ENV{VULKAN_SDK}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/graphics/window/windowlib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui/misc/cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/imgui_node_editor
)

target_include_directories(${NAME} PRIVATE ${LUAJIT_DIR}/src)


# Link against WindowLib and other dependencies
target_link_libraries(${NAME} PUBLIC
        WindowLib
        project_management
        imgui_node_editor
        nfd
        SDL3::SDL3
        csv
        ${LUAJIT_DIR}/src/libluajit.a
        m dl
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
