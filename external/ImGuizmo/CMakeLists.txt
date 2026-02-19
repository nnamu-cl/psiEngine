cmake_minimum_required(VERSION 3.20)
project(ImGuizmo)

add_library(ImGuizmo STATIC
    ImGuizmo.cpp
    GraphEditor.cpp
    ImCurveEdit.cpp
    ImGradient.cpp
    ImSequencer.cpp
)

target_include_directories(ImGuizmo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../imgui
)
