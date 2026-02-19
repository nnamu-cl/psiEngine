cmake_minimum_required(VERSION 3.20)
project(im3d)

add_library(im3d STATIC im3d.cpp)

target_include_directories(im3d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
