cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

set(MAINNAME decision_engine)
project(${MAINNAME})
set(CMAKE_CXX_STANDARD 23)

# We make use of csv-reader for reading csv files
add_subdirectory(../../external/csv-parser csv-build)
add_subdirectory(../../external/zpp_bits zpp_bits)

set(LUAJIT_DIR /home/namusanga/Documents/GitHub/LuaJIT)

# Define the files that should be included
file(GLOB_RECURSE decision_engine_files "./*.cpp" "./*.h")
list(FILTER decision_engine_files EXCLUDE REGEX ".*/cmake-build[^/]*/.*")

add_executable(${MAINNAME} ${decision_engine_files})
target_compile_options(${MAINNAME} PRIVATE -Wno-template-body -no-pie)
target_link_options(${MAINNAME} PRIVATE -no-pie -Wl,-E)
target_include_directories(${MAINNAME} PRIVATE ${LUAJIT_DIR}/src)
target_link_libraries(${MAINNAME}
        csv
        ${LUAJIT_DIR}/src/libluajit.a
        m dl
)