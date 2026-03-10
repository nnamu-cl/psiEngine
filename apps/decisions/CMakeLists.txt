cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)


set (MAINNAME decision_engine)
project (${MAINNAME})
set (CMAKE_CXX_STANDARD 23)


#Define the files that should be included
file (GLOB decision_engine_files "./*.cpp" "./*.h")

add_executable(${MAINNAME} ${decision_engine_files})