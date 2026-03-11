cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

set(NAME zzpbits)
project(${NAME} CXX)
set(CMAKE_CXX_STANDARD 20)


add_library(${NAME} zpp_bits.hpp builder.cpp)

