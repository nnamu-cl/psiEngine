# Node System Documentation

## Overview

The node system provides a Blender/Unreal-style node graph evaluation framework for controlling GameObject properties through visual programming. It features lazy evaluation with dirty flagging, automatic type conversion, and property binding.

## Architecture

### Core Components

#### 1. **NodeValue** (`NodeSystem.h`)
Type-safe variant container for all node data types:
- `float` - Single precision floating point
- `int` - Integer values
- `glm::vec2` - 2D vectors
- `glm::vec3` - 3D vectors (most common for positions/rotations)
- `glm::vec4` - 4D vectors
- `glm::mat4` - 4x4 matrices

#### 2. **TypeConverter** (`NodeSystem.h/cpp`)
Handles automatic type conversion between compatible types:
- `int` → `float`
- `float` → `vec2/vec3` (splat)
- `vec3` → `float` (magnitude)
- `vec2` ↔ `vec3`

#### 3. **InputSocket & OutputSocket** (`NodeSystem.h/cpp`)
Connection points for node data flow:

**OutputSocket:**
- Belongs to a node
- Can connect to multiple InputSockets
- Stores cached output value
- Has a name and type

**InputSocket:**
- Belongs to a node
- Can connect to ONE OutputSocket
- Has default value when not connected
- Automatically triggers node evaluation when getValue() is called

#### 4. **Node** (Base class - `NodeSystem.h/cpp`)
Base class for all nodes:
- Has multiple named inputs and outputs
- Implements `evaluate()` method
- Dirty flag for lazy evaluation
- Unique ID for identification

**Dirty Flag System:**
- Nodes are marked dirty each frame
- Evaluation only happens when dirty
- Prevents redundant calculations
- Automatically propagates downstream

#### 5. **NodeGraph** (`NodeSystem.h/cpp`)
Container and manager for all nodes:
- Factory method to create nodes
- Manages node connections
- Handles dirty flag propagation
- Type checking on connections

#### 6. **NodePropertyBinding** (`NodePropertyBinding.h/cpp`)
System for binding GameObject properties to node outputs:
- Maps GameObject properties to node outputs
- Automatically updates bound properties each frame
- Supports Position, Rotation, Scale (extensible)

## Implemented Nodes (15 Total)

### Value Nodes (`ValueNodes.h/cpp`)
1. **FloatConstantNode** - Outputs a constant float value
2. **IntConstantNode** - Outputs a constant integer value
3. **Vec3ConstantNode** - Outputs a constant vec3 value
4. **TimeNode** - Outputs elapsed time (updated externally)

### Math Nodes (`MathNodes.h/cpp`)
5. **AddNode** - A + B (works with float and vectors)
6. **SubtractNode** - A - B
7. **MultiplyNode** - A * B (supports scalar and vector)
8. **DivideNode** - A / B (with division-by-zero protection)
9. **SinNode** - sin(value)
10. **CosNode** - cos(value)

### Vector Nodes (`VectorNodes.h/cpp`)
11. **CombineVec3Node** - Combines X, Y, Z → vec3
12. **SeparateVec3Node** - Splits vec3 → X, Y, Z
13. **DotProductNode** - Dot product of two vec3s
14. **CrossProductNode** - Cross product of two vec3s
15. **LengthNode** - Magnitude of a vec3

## Integration with PsiEngine

### PsiNodeEditorLayer
The node system is integrated into `PsiNodeEditorLayer`:

```cpp
class PsiNodeEditorLayer {
    NodeGraph m_NodeGraph;                              // Node graph manager
    std::unique_ptr<NodePropertyBinding> m_PropertyBinding; // Property binding system
    float m_ElapsedTime;                                // Time for TimeNode updates

    void OnUpdate(float ts) {
        m_ElapsedTime += ts;
        m_NodeGraph.markAllDirty();      // Mark all dirty each frame
        m_PropertyBinding->updateAll();  // Update bound properties
    }
};
```

### Execution Flow (Each Frame)
1. **OnUpdate()** is called with delta time
2. All nodes are marked dirty
3. Property bindings are updated:
   - For each binding, get the connected OutputSocket
   - If the node is dirty, evaluate it
   - Apply the output value to the GameObject property
4. Node evaluation is recursive (evaluates dependencies on-demand)

## Usage Examples

### Example 1: Oscillating Y Position
```cpp
// Create nodes
auto* timeNode = graph.createNode<TimeNode>();
auto* multiplyNode = graph.createNode<MultiplyNode>();
auto* sinNode = graph.createNode<SinNode>();
auto* combineVec3 = graph.createNode<CombineVec3Node>();
auto* speedConstant = graph.createNode<FloatConstantNode>(5.0f);

// Connect: Time -> Multiply -> Sin -> Vec3.Y
graph.connect(timeNode->getOutput("Time"), multiplyNode->getInput("A"));
graph.connect(speedConstant->getOutput("Value"), multiplyNode->getInput("B"));
graph.connect(multiplyNode->getOutput("Result"), sinNode->getInput("Value"));
graph.connect(sinNode->getOutput("Result"), combineVec3->getInput("Y"));

// Bind GameObject position
binding.bind(objectId, NodePropertyBinding::PropertyType::Position,
             combineVec3->getOutput("Vector"));
```

### Example 2: Circular Motion (XZ Plane)
```cpp
auto* timeNode = graph.createNode<TimeNode>();
auto* sinNode = graph.createNode<SinNode>();
auto* cosNode = graph.createNode<CosNode>();
auto* combineVec3 = graph.createNode<CombineVec3Node>();

graph.connect(timeNode->getOutput("Time"), sinNode->getInput("Value"));
graph.connect(timeNode->getOutput("Time"), cosNode->getInput("Value"));
graph.connect(sinNode->getOutput("Result"), combineVec3->getInput("X"));
graph.connect(cosNode->getOutput("Result"), combineVec3->getInput("Z"));

binding.bind(objectId, NodePropertyBinding::PropertyType::Position,
             combineVec3->getOutput("Vector"));
```

See `NodeSystemExample.h` for more complete examples including spiral motion, pulsating scale, etc.

## How to Use in Your Code

### Setup (in PsiNodeEditorLayer)
```cpp
// Access the node graph and binding system
NodeGraph& graph = nodeEditorLayer->getNodeGraph();
NodePropertyBinding& binding = nodeEditorLayer->getPropertyBinding();

// Get GameObject ID
uint64_t objectId = scene.objects[0].id;
```

### Create and Connect Nodes
```cpp
// Create nodes
auto* nodeA = graph.createNode<FloatConstantNode>(5.0f);
auto* nodeB = graph.createNode<AddNode>();

// Connect nodes
graph.connect(nodeA->getOutput("Value"), nodeB->getInput("A"));

// Bind to GameObject property
binding.bind(objectId, NodePropertyBinding::PropertyType::Position,
             nodeB->getOutput("Result"));
```

### Update Time Nodes
```cpp
// Time node needs to be updated with elapsed time
TimeNode* timeNode = /* ... */;
timeNode->setTime(elapsedTime);  // This marks it dirty automatically
```

### Unbind Properties
```cpp
binding.unbind(objectId, NodePropertyBinding::PropertyType::Position);
```

## Performance Considerations

### Lazy Evaluation
- Nodes only evaluate when dirty
- Dependencies are evaluated recursively on-demand
- Cached results reused within the same frame

### Type Conversion
- Automatic conversion between compatible types
- Zero cost when types match exactly
- Optional conversion failures return std::nullopt

### Memory
- Nodes are heap-allocated and managed by NodeGraph
- Sockets are stored inline in nodes (no extra allocations)
- Static type checking at connection time

## Extending the System

### Adding New Node Types
1. Inherit from `Node` base class
2. In constructor, call `addInput()` and `addOutput()` for sockets
3. Implement `evaluate()` method
4. Implement `getTypeName()` for debugging

Example:
```cpp
class MyCustomNode : public Node {
public:
    MyCustomNode() {
        addInput("Input", SocketType::Float, 0.0f);
        addOutput("Output", SocketType::Float, 0.0f);
    }

    void evaluate() override {
        if (!isDirty()) return;

        float input = std::get<float>(getInput("Input")->getValue());
        float result = /* your computation */;

        getOutput("Output")->setValue(result);
        markClean();
    }

    const char* getTypeName() const override { return "My Custom Node"; }
};
```

### Adding New Property Types
1. Add to `NodePropertyBinding::PropertyType` enum
2. Add case in `updateAll()` switch statement
3. Handle the property binding in the case

### Adding New Value Types
1. Add to `NodeValue` variant
2. Update `SocketType` enum
3. Add type conversion rules in `TypeConverter::convert()`
4. Update `TypeConverter::isCompatible()`

## File Structure

```
libs/graphics/nodeGraph/
├── NodeSystem.h              - Core node system (Node, Socket, NodeGraph, TypeConverter)
├── NodeSystem.cpp
├── ValueNodes.h              - Constant and time nodes
├── ValueNodes.cpp
├── MathNodes.h               - Math operation nodes
├── MathNodes.cpp
├── VectorNodes.h             - Vector operation nodes
├── VectorNodes.cpp
├── NodePropertyBinding.h     - GameObject property binding system
├── NodePropertyBinding.cpp
├── NodeSystemExample.h       - Usage examples
└── NODE_SYSTEM_README.md     - This file
```

## Future Enhancements

Potential additions:
- More math nodes (pow, sqrt, abs, clamp, lerp, etc.)
- Matrix nodes (transform, decompose, etc.)
- Comparison nodes (greater, less, equals)
- Logic nodes (and, or, not, branch)
- Noise nodes (Perlin, Simplex)
- Easing functions (ease-in, ease-out, etc.)
- Spline/curve evaluation
- Color nodes (RGB, HSV conversion)
- Material property binding
- Mesh property binding

## Notes

- All angles are in radians
- Right-handed coordinate system (OpenGL/Vulkan convention)
- Vec3 rotation in Transform uses Euler angles (pitch, yaw, roll)
- Connections are validated at creation time (type compatibility check)
- Node IDs are unique and auto-generated
- The system is thread-safe for reading but NOT for modification
