cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)


set (MAINNAME decision_engine)
project (${MAINNAME})
set (CMAKE_CXX_STANDARD 23)


# We make use of csv-reader for reading csv files
add_subdirectory(../../external/csv-parser csv-build)
add_subdirectory(../../external/zpp_bits zpp_bits)


#Define the files that should be included
file (GLOB_RECURSE decision_engine_files "./*.cpp" "./*.h")
list(FILTER decision_engine_files EXCLUDE REGEX ".*/cmake-build[^/]*/.*")

add_executable(${MAINNAME} ${decision_engine_files})
target_link_libraries(${MAINNAME} csv)