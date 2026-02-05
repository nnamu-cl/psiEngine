# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Prerequisites

- Vulkan SDK installed, with the `VULKAN_SDK` environment variable set to its root path.
  The SDK must contain SDL3, Slang (shader compiler), GLM, and Volk headers/libraries.
- CMake 3.20+
- A C++20-capable compiler (MSVC on Windows)
- Ninja build system (used by CLion; alternatively Visual Studio generator works too)

## Build Commands

Configure and build (from the repo root, using an out-of-source build directory):

```
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

To select a specific GPU when multiple are present, pass its index as the first
argument:

```
cmake-build-debug\bin\MainApplication.exe 1
```

## Architecture Overview
The application is intended to be an SDL, Vulkan application that uses custom rendering and imGUI for UI.

The intended project structure will include two main applications that will follow a structure similar to the following:
simulation-suite/
├── CMakeLists.txt                    # Root CMake - orchestrates everything
├── CMakePresets.json                 # Build configurations
├── .github/workflows/                # CI/CD for both apps
├── cmake/
│   ├── CompilerWarnings.cmake
│   ├── Dependencies.cmake
│   └── ProjectOptions.cmake
│
├── libs/                             # Shared libraries (domain-agnostic)
│   ├── core/                         # Shared core engine architecture
│   │   ├── CMakeLists.txt
│   │   ├── include/core/
│   │   │   ├── ecs/                  # ECS framework (if shared)
│   │   │   ├── memory/               # Memory management
│   │   │   ├── threading/            # Thread pools, job system
│   │   │   └── events/               # Event system
│   │   └── src/
│   │
│   ├── math/                         # Shared math library
│   │   ├── CMakeLists.txt
│   │   ├── include/math/
│   │   │   ├── vector.hpp
│   │   │   ├── matrix.hpp
│   │   │   ├── quaternion.hpp
│   │   │   └── algorithms/           # Numerical methods, etc.
│   │   └── src/
│   │
│   └── graphics/                     # Shared rendering system
│       ├── CMakeLists.txt
│       ├── include/graphics/
│       │   ├── renderer/
│       │   ├── vulkan/               # Vulkan backend
│       │   ├── window/               # SDL window management
│       │   └── ui/                   # Shared UI components
│       └── src/
│
├── modules/                          # Application-specific modules
│   ├── physics/                      # Physics formulae library
│   │   ├── CMakeLists.txt
│   │   ├── include/physics/
│   │   │   ├── mechanics/            # Classical mechanics
│   │   │   ├── thermodynamics/
│   │   │   ├── electromagnetics/
│   │   │   └── quantum/              # If needed
│   │   └── src/
│   │
│   ├── finance/                      # Financial calculation library
│   │   ├── CMakeLists.txt
│   │   ├── include/finance/
│   │   │   ├── derivatives/          # Options, futures, etc.
│   │   │   ├── portfolio/            # Portfolio management
│   │   │   ├── risk/                 # Risk calculations
│   │   │   └── time-series/          # Market data analysis
│   │   └── src/
│   │
│   ├── psi-core/                     # PSI-specific engine code
│   │   ├── CMakeLists.txt
│   │   ├── include/psi/
│   │   │   ├── simulation/           # Physics simulation engine
│   │   │   ├── visualization/        # Physics-specific rendering
│   │   │   └── analysis/             # Results analysis
│   │   └── src/
│   │
│   └── wealth-core/                  # WealthEngine-specific code
│       ├── CMakeLists.txt
│       ├── include/wealth/
│       │   ├── engine/               # Financial calculation engine
│       │   ├── data/                 # Market data management
│       │   └── reporting/            # Financial reports/charts
│       └── src/
│
├── apps/                             # Executable applications
│   ├── psi/                          # Physics evaluation app
│   │   ├── CMakeLists.txt
│   │   ├── src/
│   │   │   └── main.cpp
│   │   └── resources/                # App-specific resources
│   │
│   └── wealth-engine/                # Financial calculation app
│       ├── CMakeLists.txt
│       ├── src/
│       │   └── main.cpp
│       └── resources/
│
├── tests/
│   ├── core-tests/
│   ├── math-tests/
│   ├── graphics-tests/
│   ├── physics-tests/
│   ├── finance-tests/
│   ├── psi-tests/                    # Integration tests for PSI
│   └── wealth-tests/                 # Integration tests for WealthEngine
│
├── benchmarks/                       # Performance tests
│   ├── math-benchmarks/
│   ├── physics-benchmarks/
│   └── finance-benchmarks/
│
├── extern/                           # Third-party dependencies
│   ├── vulkan-headers/               # As git submodules
│   ├── SDL/
│   └── ...
│
└── docs/
├── architecture.md
├── psi/
└── wealth-engine/


The above is simply a general struture to help align you with the thinking behind the project. 


