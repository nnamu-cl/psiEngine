cmake_minimum_required(VERSION 4.0)
project(ImPlotExample LANGUAGES CXX C)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

set(imgui_SOURCE_DIR ../../../external/imgui)


# Setup ImPlot
set(IMPLOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IMPLOT_SOURCE
    ${IMPLOT_SOURCE_DIR}/implot.cpp
    ${IMPLOT_SOURCE_DIR}/implot_demo.cpp
    ${IMPLOT_SOURCE_DIR}/implot_items.cpp
)
add_library(implot STATIC ${IMPLOT_SOURCE})

# Glob imgui core files (non-recursive, main directory only)
file(GLOB IMGUI_CORE_SOURCES
    ${imgui_SOURCE_DIR}/*.cpp
    ${imgui_SOURCE_DIR}/*.h
)


target_sources(implot PRIVATE
    ${IMGUI_CORE_SOURCES}
)

target_include_directories(implot PUBLIC
    ${IMPLOT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
)
