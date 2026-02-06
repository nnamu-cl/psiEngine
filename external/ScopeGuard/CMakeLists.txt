cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(ScopeGuard)

# Create ScopeGuard as a header-only INTERFACE library
add_library(ScopeGuard INTERFACE)

# Set include directories
target_include_directories(ScopeGuard INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add the header file to the target (for IDE visibility)
target_sources(ScopeGuard INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeGuard.h
)

# Group files for IDE
source_group("ThirdParty\\ScopeGuard" FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ScopeGuard.h
)
