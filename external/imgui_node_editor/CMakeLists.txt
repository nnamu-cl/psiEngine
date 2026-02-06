cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(imgui_node_editor)

# Create imgui_node_editor as a static library
add_library(imgui_node_editor STATIC
    imgui_node_editor.cpp
    imgui_node_editor_api.cpp
    imgui_canvas.cpp
    crude_json.cpp
)

# Set the imgui directory path
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../imgui)

# Include directories
target_include_directories(imgui_node_editor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
)

# Link dependencies (ScopeGuard and stb_image are header-only)
target_link_libraries(imgui_node_editor PUBLIC
    ScopeGuard
    stb_image
)

# Set C++ standard
target_compile_features(imgui_node_editor PUBLIC cxx_std_17)

# Group files for IDE
source_group("ThirdParty\\imgui_node_editor" FILES
    imgui_node_editor.cpp
    imgui_node_editor_api.cpp
    imgui_canvas.cpp
    crude_json.cpp
    imgui_node_editor.h
    imgui_node_editor_internal.h
    imgui_canvas.h
    crude_json.h
    imgui_bezier_math.h
    imgui_extra_math.h
)
