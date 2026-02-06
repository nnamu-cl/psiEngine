# ==============================================================================
# CMAKE VERSION & PROJECT SETUP
# ==============================================================================

# Require CMake 3.20 or newer. FATAL_ERROR means stop immediately if too old
# CMake 3.20 brings many modern features we'll use
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Add custom CMake modules directory to the search path
# This lets you include custom .cmake files from the cmake/ folder
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# FetchContent allows downloading dependencies at configure time (like GLM below)
include(FetchContent)

# Set the project name in a variable for reuse throughout the file
# Using variables makes it easy to rename your project in one place
set(NAME MainApplication)

# Actually create the CMake project with the name above
# This sets PROJECT_NAME, PROJECT_SOURCE_DIR, and other project-level variables
project(${NAME})


# ==============================================================================
# VULKAN SDK DETECTION
# ==============================================================================

# Check if VULKAN_SDK environment variable points to a valid directory
# $ENV{VARIABLE_NAME} accesses environment variables from your system
if (NOT IS_DIRECTORY $ENV{VULKAN_SDK})
    # If not found, warn the user but continue (you could use FATAL_ERROR to stop)
    message(WARNING "Vulkan SDK not detected, if installed please set VULKAN_SDK env var or manually provide libraries")
else()
    # If found, print a status message showing where it's located
    message(STATUS "Found Vulkan SDK in $ENV{VULKAN_SDK}")
endif()


# ==============================================================================
# INCLUDE DIRECTORIES (Where to find header files)
# ==============================================================================

# include_directories() tells the compiler where to look for #include files
# All targets in this CMakeLists.txt will search these directories

# Headers from Vulkan SDK (SDL3, Volk, VMA, glm, Slang)
include_directories($ENV{VULKAN_SDK}/include)

# External libraries bundled with this project (not from SDK)
include_directories(external/tinyobj)      # For loading .obj 3D models
include_directories(external/ktx/include)  # For loading KTX textures
include_directories(external/ktx/other_include)

# NOTE: Modern CMake prefers target_include_directories() over include_directories()
# because it's more explicit about which target needs which includes
# But include_directories() works fine for smaller projects


# ==============================================================================
# FIND REQUIRED PACKAGES
# ==============================================================================

# find_package() searches for installed libraries on your system
# REQUIRED means CMake will fail if Vulkan isn't found
# This locates the Vulkan SDK and sets variables like Vulkan_LIBRARIES
find_package(Vulkan REQUIRED)


# ==============================================================================
# PLATFORM-SPECIFIC: FETCH GLM FOR NON-WINDOWS PLATFORMS
# ==============================================================================

# GLM (OpenGL Mathematics) is a header-only math library for graphics
# On Windows, it comes with the Vulkan SDK
# On Linux/Mac, we need to download it ourselves

if (NOT WIN32)
    # FetchContent_Declare tells CMake where to download GLM from
    FetchContent_Declare(
            GLM
            GIT_REPOSITORY https://github.com/g-truc/glm    # GitHub repo
            GIT_TAG 1.0                                      # Specific version/tag
            GIT_SHALLOW ON                                   # Don't download full git history
            EXCLUDE_FROM_ALL                                 # Don't build GLM's tests/examples
            SYSTEM                                           # Treat as system library (suppress warnings)
    )
    # Actually download and make GLM available
    FetchContent_MakeAvailable(GLM)
endif()


# ==============================================================================
# BUILD KTX LIBRARY (For texture loading)
# ==============================================================================

# KTX is for loading Khronos Texture format files
# We're building it from source files included in external/ktx

# Set a variable pointing to the KTX directory for cleaner code below
set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ktx)

# List all the C source files needed to build KTX
# We manually list these because they're from a third-party library
set(KTX_SOURCES
        ${KTX_DIR}/lib/texture.c
        ${KTX_DIR}/lib/hashlist.c
        ${KTX_DIR}/lib/checkheader.c
        ${KTX_DIR}/lib/swap.c
        ${KTX_DIR}/lib/memstream.c
        ${KTX_DIR}/lib/filestream.c
        ${KTX_DIR}/lib/vkloader.c
)

# Create a static library target named "ktx" from the sources above
# STATIC means it will be compiled into your executable (not a separate .dll/.so)
add_library(ktx STATIC ${KTX_SOURCES})

# On non-Windows platforms, KTX needs to link against Vulkan
if (NOT WIN32)
    # PUBLIC means anything linking to ktx will also link to Vulkan
    target_link_libraries(ktx PUBLIC $ENV{VULKAN_SDK}/lib/libvulkan.so)
endif()


# ==============================================================================
# FIND ADDITIONAL LIBRARIES (SDL3 and Slang)
# ==============================================================================

# find_library() searches for pre-built library files
# SDL3: Cross-platform windowing and input library
# HINTS tells CMake where to look first (in the Vulkan SDK lib folder)
# REQUIRED means fail if not found
find_library(SDL_LIBRARY NAMES SDL3 HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)

# Slang: Shader compilation library
# We search for two possible names (slang or slang-compiler)
find_library(Slang_LIBRARY NAMES slang slang-compiler HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)


# ==============================================================================
# COMPILER FLAGS (Platform-specific and general)
# ==============================================================================

# CMAKE_CXX_FLAGS are flags passed to the C++ compiler on every build
# We're appending to existing flags rather than replacing them

# Platform-specific Vulkan window system defines
if(WIN32)
    # Tell Vulkan we're using Win32 for windowing
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
elseif(LINUX)
    # Tell Vulkan we're using Wayland for windowing on Linux
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
endif()

# Cross-platform defines that help with compilation
# NOMINMAX: Prevents Windows.h from defining min/max macros that conflict with std::min/max
# _USE_MATH_DEFINES: Enables M_PI and other math constants in <cmath>
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# MSVC-specific flag: Enable C++ exception handling
if(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

# Tell CMake where to put compiled executables
# ${CMAKE_BINARY_DIR} is your build folder (e.g., cmake-build-debug)
# This means .exe files go in cmake-build-debug/bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")


# ==============================================================================
# AUTOMATICALLY FIND ALL SOURCE FILES IN src/ DIRECTORY
# ==============================================================================

# file(GLOB_RECURSE ...) finds all files matching a pattern
# CONFIGURE_DEPENDS (CMake 3.12+) makes CMake check for new files on each build
# This means you can add new .cpp/.h files and they're automatically included!

# Find all C++ source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Find all header files (optional but helps IDEs show them in project tree)
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

# Print what files were found (helpful for debugging)
# This will appear in CMake output when you configure
message(STATUS "Found ${CMAKE_CURRENT_LIST_LINE} source files:")
foreach(SOURCE_FILE ${PROJECT_SOURCES})
    message(STATUS "  - ${SOURCE_FILE}")
endforeach()


# ==============================================================================
# CREATE THE EXECUTABLE TARGET
# ==============================================================================

# add_executable() creates the actual program target
# ${NAME} expands to "psiEngine"
# We include:
#   - All auto-detected sources from src/ (${PROJECT_SOURCES})
#   - All auto-detected headers (${PROJECT_HEADERS}) - helps IDE
#   - Shader file (assets/shader.slang) - so IDE shows it in project tree
add_executable(${NAME}
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        assets/shader.slang
)


# ==============================================================================
# CONFIGURE THE TARGET (Compile definitions, features, linking)
# ==============================================================================

# target_compile_definitions() adds #define preprocessor macros
# PRIVATE means only this target sees these defines (not things that link to it)

# VK_NO_PROTOTYPES: We're using Volk for Vulkan function loading, not direct linking
target_compile_definitions(${NAME} PRIVATE VK_NO_PROTOTYPES)

# Windows-specific: Disable security warnings for "unsafe" C functions
if (WIN32)
    target_compile_definitions(${NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# set_target_properties() configures various target settings
# DEBUGGER_WORKING_DIRECTORY: When debugging in IDE, run from source folder
# This makes relative paths like "assets/model.obj" work in the debugger
set_target_properties(${NAME} PROPERTIES
        DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Require C++20 standard (for features like concepts, ranges, etc.)
# PRIVATE means only this target needs C++20, not its dependencies
target_compile_features(${NAME} PRIVATE cxx_std_20)

# target_link_libraries() tells the linker what libraries this target needs
# PRIVATE means these are implementation details (not exposed to dependents)
target_link_libraries(${NAME} PRIVATE
        ${SDL_LIBRARY}      # SDL3 for windowing
        ktx                 # KTX library we built above
        ${Slang_LIBRARY}    # Slang shader compiler
)

# On non-Windows, also link GLM (on Windows it's header-only from SDK)
if (NOT WIN32)
    # glm::glm-header-only is the target name created by FetchContent
    target_link_libraries(${NAME} PRIVATE glm::glm-header-only)
endif()


# ==============================================================================
# POST-BUILD STEP: COPY ASSETS TO BUILD DIRECTORY
# ==============================================================================

# add_custom_command with POST_BUILD runs after the executable is built
# This copies the assets/ folder next to your .exe so it can find resources

add_custom_command(
        TARGET ${NAME}                                        # Run after building psiEngine
        POST_BUILD                                            # Run after linking is complete
        COMMAND ${CMAKE_COMMAND} -E copy_directory            # Use CMake's cross-platform copy
        ${CMAKE_CURRENT_SOURCE_DIR}/assets                # Source: assets/ in source tree
        $<TARGET_FILE_DIR:${NAME}>/assets                 # Destination: next to .exe in build tree
        COMMENT "Copying assets to build directory"          # Message shown during build
)

# Copy LYGIA shader includes so the runtime shader compiler can
# resolve #include "lygia/..." when using assets/ as the include root.
add_custom_command(
        TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/external/lygia
        $<TARGET_FILE_DIR:${NAME}>/assets/lygia
        COMMENT "Copying LYGIA shaders to build directory"
)

# WHY WE NEED THIS:
# Your .exe runs from cmake-build-debug/bin/
# Without this, it can't find assets/suzanne.obj
# With this, assets are copied to cmake-build-debug/bin/assets/
# So your code can use relative paths like "assets/suzanne.obj"

# NOTE: $<TARGET_FILE_DIR:${NAME}> is a "generator expression"
# It expands to the actual directory where psiEngine.exe is placed
# This works correctly with multi-config generators (Visual Studio)
# that put Debug builds in bin/Debug/ and Release builds in bin/Release/


# ==============================================================================
# SUMMARY OF HOW TO USE THIS FILE
# ==============================================================================

# ADDING NEW SOURCE FILES:
# Just create a new .cpp or .h file in the src/ folder
# Then reload the CMake project in CLion (it will show a banner)
# The file will be automatically included!

# ADDING NEW ASSETS:
# Just add files to the assets/ folder
# They'll be copied to the build directory on next build

# CHANGING PROJECT NAME:
# Change the "set(NAME psiEngine)" line at the top
# Everything else updates automatically!

# COMMON VARIABLES REFERENCE:
# ${NAME}                      = "psiEngine" (your project name)
# ${CMAKE_SOURCE_DIR}          = Root source folder (where top CMakeLists.txt is)
# ${CMAKE_CURRENT_SOURCE_DIR}  = Current CMakeLists.txt folder (same as above here)
# ${CMAKE_BINARY_DIR}          = Build folder (cmake-build-debug, etc.)
# ${CMAKE_CURRENT_BINARY_DIR}  = Current build folder
# $<TARGET_FILE_DIR:${NAME}>   = Where the executable is placed (e.g., bin/Debug)


# ==============================================================================
# ADD SUBDIRECTORIES FOR LIBRARIES AND APPLICATIONS
# ==============================================================================

# Add the graphics window library
add_subdirectory(libs/graphics/window)

# Add the PSI application
add_subdirectory(apps/psi)