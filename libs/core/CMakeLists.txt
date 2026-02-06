cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(CoreLib)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE CORE_FILES CONFIGURE_DEPENDS "include/*.h" "src/*.cpp")
add_library(CoreLib ${CORE_FILES})

target_include_directories(CoreLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    $ENV{VULKAN_SDK}/include  # For glm
)
