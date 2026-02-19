cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

set(NAME window)
project(${NAME})
set(CMAKE_CXX_STANDARD 23)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_library(SDL_LIBRARY NAMES SDL3 HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
find_library(SLANG_LIBRARY NAMES slang HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)

# WindowLib
file(GLOB_RECURSE WINDOWLIBFILES CONFIGURE_DEPENDS "windowlib/*.cpp" "windowlib/*.h")

# NodeGraph files (in sibling directory)
file(GLOB_RECURSE NODEGRAPHFILES CONFIGURE_DEPENDS "../nodeGraph/*.cpp" "../nodeGraph/*.h")

add_library(WindowLib ${WINDOWLIBFILES} ${NODEGRAPHFILES})
set(IMGUI_DIR ../../../external/imgui) #adding the imgui lib
target_include_directories(WindowLib PUBLIC
        $ENV{VULKAN_SDK}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/windowlib
        ${CMAKE_CURRENT_SOURCE_DIR}/../nodeGraph
        ${CMAKE_CURRENT_SOURCE_DIR}/../../core/include
        ${IMGUI_DIR} ${IMGUI_DIR}/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../external/imgui_node_editor
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
    PRIVATE ${SDL_LIBRARY} ${SLANG_LIBRARY} CoreLib glaze::glaze stb_image
    PUBLIC generator imgui_node_editor implot
)

if(MSVC)
    target_compile_options(WindowLib PRIVATE /Zc:preprocessor)
endif()

# OpenWindow example
file(GLOB OPENWINDOWFILES "examples/OpenWindow/*.cpp" "examples/OpenWindow/*.h")
add_executable(OpenWindow ${OPENWINDOWFILES})
target_link_libraries(OpenWindow PRIVATE WindowLib)


# Im3DDemo example
file(GLOB IM3DDEMO "examples/Im3DDemo/*.cpp" "examples/Im3DDemo/*.h")
add_executable(Im3DDemo ${IM3DDEMO})
target_link_libraries(Im3DDemo PRIVATE WindowLib im3d)



# GraphicsDemo example
file(GLOB GRAPHICSDEMOFILES "examples/GraphicsDemo/*.cpp" "examples/GraphicsDemo/*.h")
add_executable(GraphicsDemo ${GRAPHICSDEMOFILES})
target_link_libraries(GraphicsDemo PRIVATE WindowLib)


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

# Node graph tests (no Vulkan required)
add_executable(tests_nodegraph tests/test_NodeGraph.cpp)
target_link_libraries(tests_nodegraph PRIVATE WindowLib Catch2::Catch2WithMain)

# Node deletion tests (no Vulkan required)
add_executable(tests_nodedeletion tests/test_NodeDeletion.cpp)
target_link_libraries(tests_nodedeletion PRIVATE WindowLib Catch2::Catch2WithMain)

# Graph nodes tests (no Vulkan required, tests LineGraphNode and visualization nodes)
add_executable(tests_graphnodes tests/test_GraphNodes.cpp)
target_link_libraries(tests_graphnodes PRIVATE WindowLib Catch2::Catch2WithMain)

# LineRenderer component tests (no Vulkan required, tests LineRenderer component)
add_executable(tests_linerenderer tests/test_LineRenderer.cpp)
target_link_libraries(tests_linerenderer PRIVATE WindowLib Catch2::Catch2WithMain)

# Material system tests (no Vulkan required, tests MaterialProperties and enums)
add_executable(tests_materials tests/test_Materials.cpp)
target_link_libraries(tests_materials PRIVATE WindowLib Catch2::Catch2WithMain)

# LineRendererNode tests (no Vulkan required, tests LineRendererNode and node graph)
add_executable(tests_linerenderernode tests/test_LineRendererNode.cpp)
target_link_libraries(tests_linerenderernode PRIVATE WindowLib Catch2::Catch2WithMain)

# LineProperties and LineRendererData tests (no Vulkan required, tests data structures)
add_executable(tests_lineproperties tests/test_LineProperties.cpp)
target_link_libraries(tests_lineproperties PRIVATE WindowLib Catch2::Catch2WithMain)

# PipelineManager tests (no Vulkan required, tests PipelineVariantKey and hash function)
add_executable(tests_pipelinemanager tests/test_PipelineManager.cpp)
target_link_libraries(tests_pipelinemanager PRIVATE WindowLib Catch2::Catch2WithMain)

# LineRenderer Integration tests (no Vulkan required, tests node + component + material integration)
add_executable(tests_linerenderer_integration tests/test_LineRendererIntegration.cpp)
target_link_libraries(tests_linerenderer_integration PRIVATE WindowLib Catch2::Catch2WithMain)

# Pipeline Integration tests (no Vulkan required, tests material-pipeline selection logic)
add_executable(tests_pipeline_integration tests/test_PipelineIntegration.cpp)
target_link_libraries(tests_pipeline_integration PRIVATE WindowLib Catch2::Catch2WithMain)

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


# Also copy assets for OpenWindow example
add_custom_command(
        TARGET GraphicsDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SRC} $<TARGET_FILE_DIR:GraphicsDemo>/libs/graphics/window/assets
        COMMENT "Copying assets to OpenWindow executable directory"
)



# GraphDemo example
file(GLOB GRAPHDEMOFILES "examples/GraphDemo/*.cpp" "examples/GraphDemo/*.h")
add_executable(GraphDemo ${GRAPHDEMOFILES})
target_link_libraries(GraphDemo PRIVATE WindowLib)

# Copy LYGIA shader includes for GraphDemo
add_custom_command(
        TARGET GraphDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${LYGIA_SRC} $<TARGET_FILE_DIR:GraphDemo>/assets/lygia
        COMMENT "Copying LYGIA shaders to GraphDemo build directory"
)

# Also copy assets for GraphDemo example
add_custom_command(
        TARGET GraphDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SRC} $<TARGET_FILE_DIR:GraphDemo>/libs/graphics/window/assets
        COMMENT "Copying assets to GraphDemo executable directory"
)



# Add the project management here
add_subdirectory(../../project_management ${CMAKE_BINARY_DIR}/project_managment)

# SavingDemo example
file(GLOB SAVINGDEMO "examples/SavingDemo/*.cpp" "examples/SavingDemo/*.h")
add_executable(SavingDemo ${SAVINGDEMO})
target_link_libraries(SavingDemo PRIVATE WindowLib project_management)