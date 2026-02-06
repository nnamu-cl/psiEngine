# PSI Application

Physics Simulation Interface - A Vulkan-based application for physics evaluation and simulation.

## Structure

```
apps/psi/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
└── src/
    ├── main.h               # Main header
    ├── main.cpp             # Application entry point
    └── layers/
        ├── PsiUILayer.h     # UI layer header
        └── PsiUILayer.cpp   # UI layer implementation
```

## Features

- **Window Management**: SDL3-based window with Vulkan rendering
- **UI System**: ImGui-based interface with custom PSI control panel
- **Game World**: DefaultGameWorld layer for 3D rendering
- **Custom UI Layer**: PsiUILayer for simulation controls and statistics

## Building

From the project root:

```bash
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

## Running

```bash
cmake-build-debug/bin/PsiApplication.exe
```

## Dependencies

- **WindowLib**: Graphics window library from `libs/graphics/window`
- **SDL3**: Window and input handling
- **Vulkan**: Graphics API
- **ImGui**: Immediate mode GUI
- **GLM**: Mathematics library
- **Slang**: Shader compilation

## UI Components

### Control Panel
- Simulation play/pause controls
- Time scale adjustment
- Physics parameter configuration (gravity, air resistance)
- Quick action buttons

### Simulation Statistics
- FPS and frame time monitoring
- Active object count (placeholder)
- Memory usage tracking (placeholder)

### Menu Bar
- View menu for toggling panels
- Simulation menu for quick actions

## Next Steps

1. Connect UI controls to actual physics simulation backend
2. Implement simulation reset functionality
3. Add memory usage tracking
4. Integrate with physics calculation modules
5. Add visualization for simulation results
