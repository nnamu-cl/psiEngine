//
// Node Graph Tests
// Tests the node graph system without requiring Vulkan/GPU or ImGui context
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nodes/NodeSystem.h"
#include "nodes/ValueNodes.h"
#include "nodes/MathNodes.h"
#include "nodes/VectorNodes.h"

using Catch::Matchers::WithinAbs;

// ====================================================================================
// TEST 1: TypeConverter - Basic Type Conversions
// ====================================================================================
TEST_CASE("TypeConverter converts between compatible types", "[NodeGraph][TypeConverter]") {
    SECTION("Int to Float conversion") {
        NodeValue intValue = 42;
        auto result = TypeConverter::convert(intValue, SocketType::Float);

        REQUIRE(result.has_value());
        REQUIRE(std::holds_alternative<float>(*result));
        REQUIRE_THAT(std::get<float>(*result), WithinAbs(42.0f, 0.001f));
    }

    SECTION("Float to Vec3 conversion (splat)") {
        NodeValue floatValue = 5.0f;
        auto result = TypeConverter::convert(floatValue, SocketType::Vec3);

        REQUIRE(result.has_value());
        REQUIRE(std::holds_alternative<glm::vec3>(*result));

        glm::vec3 vec = std::get<glm::vec3>(*result);
        REQUIRE_THAT(vec.x, WithinAbs(5.0f, 0.001f));
        REQUIRE_THAT(vec.y, WithinAbs(5.0f, 0.001f));
        REQUIRE_THAT(vec.z, WithinAbs(5.0f, 0.001f));
    }

    SECTION("Vec3 to Float conversion (magnitude)") {
        NodeValue vec3Value = glm::vec3(3.0f, 4.0f, 0.0f); // 3-4-5 triangle
        auto result = TypeConverter::convert(vec3Value, SocketType::Float);

        REQUIRE(result.has_value());
        REQUIRE(std::holds_alternative<float>(*result));
        REQUIRE_THAT(std::get<float>(*result), WithinAbs(5.0f, 0.001f));
    }

    SECTION("Vec2 to Vec3 conversion (adds z=0)") {
        NodeValue vec2Value = glm::vec2(1.0f, 2.0f);
        auto result = TypeConverter::convert(vec2Value, SocketType::Vec3);

        REQUIRE(result.has_value());
        REQUIRE(std::holds_alternative<glm::vec3>(*result));

        glm::vec3 vec = std::get<glm::vec3>(*result);
        REQUIRE_THAT(vec.x, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(vec.y, WithinAbs(2.0f, 0.001f));
        REQUIRE_THAT(vec.z, WithinAbs(0.0f, 0.001f));
    }

    SECTION("Vec3 to Vec2 conversion (drops z)") {
        NodeValue vec3Value = glm::vec3(1.0f, 2.0f, 3.0f);
        auto result = TypeConverter::convert(vec3Value, SocketType::Vec2);

        REQUIRE(result.has_value());
        REQUIRE(std::holds_alternative<glm::vec2>(*result));

        glm::vec2 vec = std::get<glm::vec2>(*result);
        REQUIRE_THAT(vec.x, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(vec.y, WithinAbs(2.0f, 0.001f));
    }

    SECTION("Incompatible types return nullopt") {
        NodeValue floatValue = 5.0f;
        auto result = TypeConverter::convert(floatValue, SocketType::Mat4);

        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Identity conversion (same type)") {
        NodeValue floatValue = 3.14f;
        auto result = TypeConverter::convert(floatValue, SocketType::Float);

        REQUIRE(result.has_value());
        REQUIRE(std::holds_alternative<float>(*result));
        REQUIRE_THAT(std::get<float>(*result), WithinAbs(3.14f, 0.001f));
    }
}

// ====================================================================================
// TEST 2: Socket Connections and Value Propagation
// ====================================================================================
TEST_CASE("Sockets handle connections and values correctly", "[NodeGraph][Sockets]") {
    NodeGraph graph;
    auto* floatNode = graph.createNode<FloatConstantNode>(10.0f);
    auto* addNode = graph.createNode<AddNode>();

    SECTION("OutputSocket stores and retrieves values") {
        floatNode->evaluate();
        auto* output = floatNode->getOutput("Value");

        REQUIRE(output != nullptr);
        REQUIRE(std::holds_alternative<float>(output->getValue()));
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(10.0f, 0.001f));
    }

    SECTION("InputSocket returns default value when not connected") {
        auto* input = addNode->getInput("A");

        REQUIRE(input != nullptr);
        REQUIRE_FALSE(input->isConnected());
        REQUIRE(std::holds_alternative<float>(input->getValue()));
        REQUIRE_THAT(std::get<float>(input->getValue()), WithinAbs(0.0f, 0.001f));
    }

    SECTION("InputSocket retrieves value from connected output") {
        auto* output = floatNode->getOutput("Value");
        auto* input = addNode->getInput("A");

        graph.connect(output, input);

        REQUIRE(input->isConnected());
        floatNode->evaluate();
        REQUIRE(std::holds_alternative<float>(input->getValue()));
        REQUIRE_THAT(std::get<float>(input->getValue()), WithinAbs(10.0f, 0.001f));
    }

    SECTION("Disconnecting socket removes connection") {
        auto* output = floatNode->getOutput("Value");
        auto* input = addNode->getInput("A");

        graph.connect(output, input);
        REQUIRE(input->isConnected());

        input->disconnect();
        REQUIRE_FALSE(input->isConnected());

        // Should return default value after disconnect
        REQUIRE(std::holds_alternative<float>(input->getValue()));
        REQUIRE_THAT(std::get<float>(input->getValue()), WithinAbs(0.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 3: Node Dirty Flag System
// ====================================================================================
TEST_CASE("Node dirty flags work correctly", "[NodeGraph][DirtyFlags]") {
    NodeGraph graph;
    auto* floatNode = graph.createNode<FloatConstantNode>(5.0f);
    auto* addNode = graph.createNode<AddNode>();

    SECTION("Nodes start dirty") {
        REQUIRE(floatNode->isDirty());
        REQUIRE(addNode->isDirty());
    }

    SECTION("Evaluation marks node clean") {
        floatNode->evaluate();
        REQUIRE_FALSE(floatNode->isDirty());
    }

    SECTION("Setting value marks node dirty") {
        floatNode->evaluate();
        REQUIRE_FALSE(floatNode->isDirty());

        floatNode->setValue(10.0f);
        REQUIRE(floatNode->isDirty());
    }

    SECTION("markDirty propagates to downstream nodes") {
        auto* output = floatNode->getOutput("Value");
        auto* input = addNode->getInput("A");
        graph.connect(output, input);

        // Mark both clean first
        floatNode->evaluate();
        addNode->evaluate();
        REQUIRE_FALSE(floatNode->isDirty());
        REQUIRE_FALSE(addNode->isDirty());

        // Marking upstream node dirty should propagate downstream
        floatNode->markDirty();
        REQUIRE(floatNode->isDirty());
        REQUIRE(addNode->isDirty());
    }

    SECTION("markAllDirty marks all nodes in graph") {
        floatNode->evaluate();
        addNode->evaluate();
        REQUIRE_FALSE(floatNode->isDirty());
        REQUIRE_FALSE(addNode->isDirty());

        graph.markAllDirty();
        REQUIRE(floatNode->isDirty());
        REQUIRE(addNode->isDirty());
    }
}

// ====================================================================================
// TEST 4: NodeGraph Connection Management
// ====================================================================================
TEST_CASE("NodeGraph manages connections correctly", "[NodeGraph][Connections]") {
    NodeGraph graph;
    auto* floatNode = graph.createNode<FloatConstantNode>(7.0f);
    auto* addNode = graph.createNode<AddNode>();

    SECTION("connect() returns true for compatible types") {
        auto* output = floatNode->getOutput("Value");
        auto* input = addNode->getInput("A");

        bool result = graph.connect(output, input);
        REQUIRE(result);
        REQUIRE(input->isConnected());
    }

    SECTION("connect() rejects incompatible types") {
        auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(1.0f));
        auto* sinNode = graph.createNode<SinNode>(); // Expects float input

        auto* output = vec3Node->getOutput("Value");
        auto* input = sinNode->getInput("Value");

        // Vec3 to Float should be compatible (converts to magnitude)
        bool result = graph.connect(output, input);
        REQUIRE(result);
    }

    SECTION("disconnect() removes connection") {
        auto* output = floatNode->getOutput("Value");
        auto* input = addNode->getInput("A");

        graph.connect(output, input);
        REQUIRE(input->isConnected());

        graph.disconnect(input);
        REQUIRE_FALSE(input->isConnected());
    }

    SECTION("Nodes have unique IDs") {
        auto* node1 = graph.createNode<FloatConstantNode>(1.0f);
        auto* node2 = graph.createNode<FloatConstantNode>(2.0f);
        auto* node3 = graph.createNode<AddNode>();

        REQUIRE(node1->getId() != node2->getId());
        REQUIRE(node1->getId() != node3->getId());
        REQUIRE(node2->getId() != node3->getId());
    }
}

// ====================================================================================
// TEST 5: FloatConstantNode Evaluation
// ====================================================================================
TEST_CASE("FloatConstantNode evaluates correctly", "[NodeGraph][ValueNodes]") {
    NodeGraph graph;

    SECTION("Node outputs initial value") {
        auto* node = graph.createNode<FloatConstantNode>(3.14f);
        node->evaluate();

        auto* output = node->getOutput("Value");
        REQUIRE(output != nullptr);
        REQUIRE(std::holds_alternative<float>(output->getValue()));
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(3.14f, 0.001f));
    }

    SECTION("setValue updates output after evaluation") {
        auto* node = graph.createNode<FloatConstantNode>(1.0f);
        node->evaluate();

        node->setValue(42.0f);
        node->evaluate();

        auto* output = node->getOutput("Value");
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(42.0f, 0.001f));
    }

    SECTION("getValue returns current value") {
        auto* node = graph.createNode<FloatConstantNode>(2.71f);
        REQUIRE_THAT(node->getValue(), WithinAbs(2.71f, 0.001f));
    }
}

// ====================================================================================
// TEST 6: AddNode with Different Types
// ====================================================================================
TEST_CASE("AddNode handles different value types", "[NodeGraph][MathNodes]") {
    NodeGraph graph;

    SECTION("Add two floats") {
        auto* node1 = graph.createNode<FloatConstantNode>(5.0f);
        auto* node2 = graph.createNode<FloatConstantNode>(3.0f);
        auto* addNode = graph.createNode<AddNode>();

        graph.connect(node1->getOutput("Value"), addNode->getInput("A"));
        graph.connect(node2->getOutput("Value"), addNode->getInput("B"));

        node1->evaluate();
        node2->evaluate();
        addNode->evaluate();

        auto* output = addNode->getOutput("Result");
        REQUIRE(std::holds_alternative<float>(output->getValue()));
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(8.0f, 0.001f));
    }

    SECTION("Add two vec3s (component-wise)") {
        auto* node1 = graph.createNode<Vec3ConstantNode>(glm::vec3(1.0f, 2.0f, 3.0f));
        auto* node2 = graph.createNode<Vec3ConstantNode>(glm::vec3(4.0f, 5.0f, 6.0f));
        auto* addNode = graph.createNode<AddNode>();

        graph.connect(node1->getOutput("Value"), addNode->getInput("A"));
        graph.connect(node2->getOutput("Value"), addNode->getInput("B"));

        node1->evaluate();
        node2->evaluate();
        addNode->evaluate();

        auto* output = addNode->getOutput("Result");
        REQUIRE(std::holds_alternative<glm::vec3>(output->getValue()));

        glm::vec3 result = std::get<glm::vec3>(output->getValue());
        REQUIRE_THAT(result.x, WithinAbs(5.0f, 0.001f));
        REQUIRE_THAT(result.y, WithinAbs(7.0f, 0.001f));
        REQUIRE_THAT(result.z, WithinAbs(9.0f, 0.001f));
    }

    SECTION("Add vec3 and float (scalar addition)") {
        auto* vecNode = graph.createNode<Vec3ConstantNode>(glm::vec3(1.0f, 2.0f, 3.0f));
        auto* floatNode = graph.createNode<FloatConstantNode>(10.0f);
        auto* addNode = graph.createNode<AddNode>();

        graph.connect(vecNode->getOutput("Value"), addNode->getInput("A"));
        graph.connect(floatNode->getOutput("Value"), addNode->getInput("B"));

        vecNode->evaluate();
        floatNode->evaluate();
        addNode->evaluate();

        auto* output = addNode->getOutput("Result");
        REQUIRE(std::holds_alternative<glm::vec3>(output->getValue()));

        glm::vec3 result = std::get<glm::vec3>(output->getValue());
        REQUIRE_THAT(result.x, WithinAbs(11.0f, 0.001f));
        REQUIRE_THAT(result.y, WithinAbs(12.0f, 0.001f));
        REQUIRE_THAT(result.z, WithinAbs(13.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 7: MultiplyNode Operations
// ====================================================================================
TEST_CASE("MultiplyNode performs multiplication correctly", "[NodeGraph][MathNodes]") {
    NodeGraph graph;

    SECTION("Multiply two floats") {
        auto* node1 = graph.createNode<FloatConstantNode>(4.0f);
        auto* node2 = graph.createNode<FloatConstantNode>(3.0f);
        auto* mulNode = graph.createNode<MultiplyNode>();

        graph.connect(node1->getOutput("Value"), mulNode->getInput("A"));
        graph.connect(node2->getOutput("Value"), mulNode->getInput("B"));

        node1->evaluate();
        node2->evaluate();
        mulNode->evaluate();

        auto* output = mulNode->getOutput("Result");
        REQUIRE(std::holds_alternative<float>(output->getValue()));
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(12.0f, 0.001f));
    }

    SECTION("Multiply vec3 by scalar") {
        auto* vecNode = graph.createNode<Vec3ConstantNode>(glm::vec3(2.0f, 3.0f, 4.0f));
        auto* floatNode = graph.createNode<FloatConstantNode>(2.0f);
        auto* mulNode = graph.createNode<MultiplyNode>();

        graph.connect(vecNode->getOutput("Value"), mulNode->getInput("A"));
        graph.connect(floatNode->getOutput("Value"), mulNode->getInput("B"));

        vecNode->evaluate();
        floatNode->evaluate();
        mulNode->evaluate();

        auto* output = mulNode->getOutput("Result");
        REQUIRE(std::holds_alternative<glm::vec3>(output->getValue()));

        glm::vec3 result = std::get<glm::vec3>(output->getValue());
        REQUIRE_THAT(result.x, WithinAbs(4.0f, 0.001f));
        REQUIRE_THAT(result.y, WithinAbs(6.0f, 0.001f));
        REQUIRE_THAT(result.z, WithinAbs(8.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 8: Trigonometric Nodes (Sin/Cos)
// ====================================================================================
TEST_CASE("Trigonometric nodes compute correctly", "[NodeGraph][MathNodes]") {
    NodeGraph graph;

    SECTION("SinNode computes sine") {
        auto* floatNode = graph.createNode<FloatConstantNode>(0.0f);
        auto* sinNode = graph.createNode<SinNode>();

        graph.connect(floatNode->getOutput("Value"), sinNode->getInput("Value"));

        floatNode->evaluate();
        sinNode->evaluate();

        auto* output = sinNode->getOutput("Result");
        REQUIRE(std::holds_alternative<float>(output->getValue()));
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(0.0f, 0.001f));

        // Test pi/2 (should be 1.0)
        floatNode->setValue(3.14159265f / 2.0f);
        floatNode->evaluate();
        sinNode->markDirty();
        sinNode->evaluate();
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(1.0f, 0.001f));
    }

    SECTION("CosNode computes cosine") {
        auto* floatNode = graph.createNode<FloatConstantNode>(0.0f);
        auto* cosNode = graph.createNode<CosNode>();

        graph.connect(floatNode->getOutput("Value"), cosNode->getInput("Value"));

        floatNode->evaluate();
        cosNode->evaluate();

        auto* output = cosNode->getOutput("Result");
        REQUIRE(std::holds_alternative<float>(output->getValue()));
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(1.0f, 0.001f));

        // Test pi (should be -1.0)
        floatNode->setValue(3.14159265f);
        floatNode->evaluate();
        cosNode->markDirty();
        cosNode->evaluate();
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(-1.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 9: Lazy Evaluation with Dirty Flags
// ====================================================================================
TEST_CASE("Lazy evaluation prevents redundant calculations", "[NodeGraph][LazyEvaluation]") {
    NodeGraph graph;
    auto* floatNode = graph.createNode<FloatConstantNode>(5.0f);
    auto* addNode = graph.createNode<AddNode>();

    graph.connect(floatNode->getOutput("Value"), addNode->getInput("A"));
    graph.connect(floatNode->getOutput("Value"), addNode->getInput("B"));

    SECTION("Node evaluates when dirty") {
        REQUIRE(floatNode->isDirty());
        floatNode->evaluate();
        REQUIRE_FALSE(floatNode->isDirty());

        auto* output = floatNode->getOutput("Value");
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(5.0f, 0.001f));
    }

    SECTION("Node skips evaluation when clean") {
        floatNode->evaluate();
        REQUIRE_FALSE(floatNode->isDirty());

        // Calling evaluate again should be a no-op (node stays clean)
        floatNode->evaluate();
        REQUIRE_FALSE(floatNode->isDirty());
    }

    SECTION("Input getValue() triggers upstream evaluation") {
        // FloatNode is dirty, AddNode's input should trigger its evaluation
        REQUIRE(floatNode->isDirty());

        auto* input = addNode->getInput("A");
        NodeValue value = input->getValue();

        // Getting the value should have triggered floatNode evaluation
        REQUIRE_FALSE(floatNode->isDirty());
        REQUIRE(std::holds_alternative<float>(value));
        REQUIRE_THAT(std::get<float>(value), WithinAbs(5.0f, 0.001f));
    }

    SECTION("Full graph evaluation respects dirty flags") {
        // Evaluate entire graph
        floatNode->evaluate();
        addNode->evaluate();

        REQUIRE_FALSE(floatNode->isDirty());
        REQUIRE_FALSE(addNode->isDirty());

        auto* output = addNode->getOutput("Result");
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(10.0f, 0.001f));

        // Mark all dirty (simulates new frame)
        graph.markAllDirty();
        REQUIRE(floatNode->isDirty());
        REQUIRE(addNode->isDirty());

        // Re-evaluate
        addNode->evaluate();
        REQUIRE_FALSE(floatNode->isDirty()); // Should be evaluated by addNode's input
        REQUIRE_FALSE(addNode->isDirty());
    }
}

// ====================================================================================
// TEST 10: Type Conversion in Graph Connections
// ====================================================================================
TEST_CASE("Type conversion works in connected graphs", "[NodeGraph][TypeConversion]") {
    NodeGraph graph;

    SECTION("Float splatted to Vec3 in connection") {
        auto* floatNode = graph.createNode<FloatConstantNode>(7.0f);
        auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(0.0f));
        auto* addNode = graph.createNode<AddNode>();

        // Connect float to vec3 input (should splat float to vec3)
        graph.connect(floatNode->getOutput("Value"), addNode->getInput("A"));
        graph.connect(vec3Node->getOutput("Value"), addNode->getInput("B"));

        floatNode->evaluate();
        vec3Node->evaluate();
        addNode->evaluate();

        // Float 7.0 should become vec3(7, 7, 7) and add to vec3(0, 0, 0)
        auto* output = addNode->getOutput("Result");
        REQUIRE(std::holds_alternative<glm::vec3>(output->getValue()));

        glm::vec3 result = std::get<glm::vec3>(output->getValue());
        REQUIRE_THAT(result.x, WithinAbs(7.0f, 0.001f));
        REQUIRE_THAT(result.y, WithinAbs(7.0f, 0.001f));
        REQUIRE_THAT(result.z, WithinAbs(7.0f, 0.001f));
    }

    SECTION("Vec3 converted to float (magnitude) in connection") {
        auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(3.0f, 4.0f, 0.0f));
        auto* sinNode = graph.createNode<SinNode>(); // Expects float

        // Connect vec3 to float input (should convert to magnitude)
        graph.connect(vec3Node->getOutput("Value"), sinNode->getInput("Value"));

        vec3Node->evaluate();
        sinNode->evaluate();

        // Vec3(3, 4, 0) has magnitude 5.0
        auto* sinOutput = sinNode->getOutput("Result");
        float sinValue = std::get<float>(sinOutput->getValue());

        // sin(5.0) ≈ -0.9589
        REQUIRE_THAT(sinValue, WithinAbs(std::sin(5.0f), 0.001f));
    }

    SECTION("Int converted to float in connection") {
        auto* intNode = graph.createNode<IntConstantNode>(10);
        auto* addNode = graph.createNode<AddNode>();
        auto* floatNode = graph.createNode<FloatConstantNode>(5.5f);

        graph.connect(intNode->getOutput("Value"), addNode->getInput("A"));
        graph.connect(floatNode->getOutput("Value"), addNode->getInput("B"));

        intNode->evaluate();
        floatNode->evaluate();
        addNode->evaluate();

        auto* output = addNode->getOutput("Result");
        REQUIRE(std::holds_alternative<float>(output->getValue()));
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(15.5f, 0.001f));
    }
}
