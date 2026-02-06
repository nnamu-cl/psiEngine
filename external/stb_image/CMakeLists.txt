cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(stb_image)

# Create stb_image as a header-only INTERFACE library
add_library(stb_image INTERFACE)

# Set include directories
target_include_directories(stb_image INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add the header file to the target (for IDE visibility)
target_sources(stb_image INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h
)

# Group files for IDE
source_group("ThirdParty\\stb_image" FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h
)
