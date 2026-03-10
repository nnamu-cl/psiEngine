cmake_minimum_required(VERSION 4.0)



project(generator LANGUAGES CXX)

# GML - Header-only math library
add_library(gml INTERFACE)


target_include_directories(gml INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/gml/include
)
target_compile_features(gml INTERFACE cxx_std_20)

# Generator - Mesh generation library
file(GLOB GENERATOR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(generator STATIC ${GENERATOR_SOURCES})

target_compile_options(generator PUBLIC -Wno-template-body)


target_include_directories(generator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(generator PUBLIC gml)

target_compile_features(generator PUBLIC cxx_std_20)

# MSVC specific flags
if(MSVC)
    target_compile_options(generator PRIVATE /W4)
endif()
