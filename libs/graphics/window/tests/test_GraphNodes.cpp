//
// Graph Nodes Tests
// Tests the LineGraphNode and related visualization nodes
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nodes/NodeSystem.h"
#include "nodes/GraphNodes.h"
#include "nodes/ObjectNodes.h"
#include "nodes/ValueNodes.h"

using Catch::Matchers::WithinAbs;

// ====================================================================================
// TEST 1: LineGraphNode Initialization
// ====================================================================================
TEST_CASE("LineGraphNode initializes with correct defaults", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* lineGraph = graph.createNode<LineGraphNode>();

    SECTION("Default values are set correctly") {
        REQUIRE(lineGraph->maxCount == 100);
        REQUIRE(lineGraph->points.capacity == 100);
        REQUIRE(lineGraph->points.buffer.empty());
        REQUIRE(lineGraph->drawGraph == false);
        REQUIRE(lineGraph->trackTransform == nullptr);
        REQUIRE(lineGraph->selectedAxis == LineGraphNode::Axis::X);
    }

    SECTION("Node has correct type name") {
        REQUIRE(std::string(lineGraph->getTypeName()) == "Line Graph");
    }

    SECTION("Node has graph reference") {
        REQUIRE(lineGraph->graph == &graph);
    }
}

// ====================================================================================
// TEST 2: Evaluate Without TransformNode
// ====================================================================================
TEST_CASE("LineGraphNode evaluate() handles null trackTransform safely", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* lineGraph = graph.createNode<LineGraphNode>();

    SECTION("Evaluate with no trackTransform does not crash") {
        REQUIRE(lineGraph->trackTransform == nullptr);
        REQUIRE_NOTHROW(lineGraph->evaluate());
    }



    SECTION("Points buffer remains empty without trackTransform") {
        lineGraph->evaluate();
        REQUIRE(lineGraph->points.buffer.empty());
    }
}

// ====================================================================================
// TEST 3: Track TransformNode X Axis
// ====================================================================================
TEST_CASE("LineGraphNode tracks TransformNode X axis correctly", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* transformNode = graph.createNode<TransformNode>();
    auto* lineGraph = graph.createNode<LineGraphNode>();

    // Set up tracking
    lineGraph->trackTransform = transformNode;
    lineGraph->selectedAxis = LineGraphNode::Axis::X;

    SECTION("Single evaluation captures X value") {
        // Set transform position
        auto* posInput = transformNode->getInput("Pos");
        transformNode->evaluate();

        // The default Pos is vec3(1.0f, 1.0f, 1.0f)
        lineGraph->evaluate();

        REQUIRE(lineGraph->points.buffer.size() == 1);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(1.0f, 0.001f));
    }

    SECTION("Multiple evaluations accumulate X values") {
        transformNode->evaluate();
        lineGraph->evaluate();

        // Change position and evaluate again
        auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(2.0f, 3.0f, 4.0f));
        graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

        vec3Node->evaluate();
        transformNode->evaluate();
        lineGraph->evaluate();

        REQUIRE(lineGraph->points.buffer.size() == 2);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[1], WithinAbs(2.0f, 0.001f));
    }

    SECTION("Tracks changing X values over time") {
        for (int i = 0; i < 5; i++) {
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(static_cast<float>(i), 0.0f, 0.0f));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            vec3Node->evaluate();
            
            transformNode->evaluate();
            
            lineGraph->evaluate();
        }

        REQUIRE(lineGraph->points.buffer.size() == 5);
        for (int i = 0; i < 5; i++) {
            REQUIRE_THAT(lineGraph->points[i], WithinAbs(static_cast<float>(i), 0.001f));
        }
    }
}

// ====================================================================================
// TEST 4: Track TransformNode Y Axis
// ====================================================================================
TEST_CASE("LineGraphNode tracks TransformNode Y axis correctly", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* transformNode = graph.createNode<TransformNode>();
    auto* lineGraph = graph.createNode<LineGraphNode>();

    lineGraph->trackTransform = transformNode;
    lineGraph->selectedAxis = LineGraphNode::Axis::Y;

    SECTION("Captures Y component from position") {
        auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(1.0f, 5.5f, 3.0f));
        graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

        vec3Node->evaluate();
        transformNode->evaluate();
        lineGraph->evaluate();

        REQUIRE(lineGraph->points.buffer.size() == 1);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(5.5f, 0.001f));
    }

    SECTION("Multiple Y values are tracked") {
        float yValues[] = {1.0f, 2.5f, 3.7f, 4.2f};

        for (float y : yValues) {
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(0.0f, y, 0.0f));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            vec3Node->evaluate();
            
            transformNode->evaluate();
            
            lineGraph->evaluate();
        }

        REQUIRE(lineGraph->points.buffer.size() == 4);
        for (size_t i = 0; i < 4; i++) {
            REQUIRE_THAT(lineGraph->points[i], WithinAbs(yValues[i], 0.001f));
        }
    }
}

// ====================================================================================
// TEST 5: Track TransformNode Z Axis
// ====================================================================================
TEST_CASE("LineGraphNode tracks TransformNode Z axis correctly", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* transformNode = graph.createNode<TransformNode>();
    auto* lineGraph = graph.createNode<LineGraphNode>();

    lineGraph->trackTransform = transformNode;
    lineGraph->selectedAxis = LineGraphNode::Axis::Z;

    SECTION("Captures Z component from position") {
        auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(1.0f, 2.0f, 7.3f));
        graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

        vec3Node->evaluate();
        transformNode->evaluate();
        lineGraph->evaluate();

        REQUIRE(lineGraph->points.buffer.size() == 1);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(7.3f, 0.001f));
    }

    SECTION("Negative Z values are tracked correctly") {
        float zValues[] = {-1.0f, -5.5f, 0.0f, 3.3f, -2.2f};

        for (float z : zValues) {
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(0.0f, 0.0f, z));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            vec3Node->evaluate();
            
            transformNode->evaluate();
            
            lineGraph->evaluate();
        }

        REQUIRE(lineGraph->points.buffer.size() == 5);
        for (size_t i = 0; i < 5; i++) {
            REQUIRE_THAT(lineGraph->points[i], WithinAbs(zValues[i], 0.001f));
        }
    }
}

// ====================================================================================
// TEST 6: CircularBuffer Capacity Management
// ====================================================================================
TEST_CASE("LineGraphNode respects maxCount capacity", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* transformNode = graph.createNode<TransformNode>();
    auto* lineGraph = graph.createNode<LineGraphNode>();

    lineGraph->trackTransform = transformNode;
    lineGraph->selectedAxis = LineGraphNode::Axis::X;
    lineGraph->maxCount = 5; // Set small capacity for testing

    SECTION("Buffer does not exceed maxCount") {
        // Push 10 values, but buffer should only keep last 5
        for (int i = 0; i < 10; i++) {
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(static_cast<float>(i), 0.0f, 0.0f));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            vec3Node->evaluate();
            
            transformNode->evaluate();
            
            lineGraph->evaluate();
        }

        REQUIRE(lineGraph->points.buffer.size() == 5);
        REQUIRE(lineGraph->points.capacity == 5);
    }

    SECTION("Oldest values are removed when capacity exceeded") {
        // Push 8 values with capacity of 5
        for (int i = 0; i < 8; i++) {
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(static_cast<float>(i * 10), 0.0f, 0.0f));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            vec3Node->evaluate();
            
            transformNode->evaluate();
            
            lineGraph->evaluate();
        }

        REQUIRE(lineGraph->points.buffer.size() == 5);
        // Should contain values from last 5 iterations: 30, 40, 50, 60, 70
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(30.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[1], WithinAbs(40.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[2], WithinAbs(50.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[3], WithinAbs(60.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[4], WithinAbs(70.0f, 0.001f));
    }

    SECTION("Capacity can be changed and affects buffer") {
        // Start with capacity 5, fill it
        for (int i = 0; i < 5; i++) {
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(static_cast<float>(i), 0.0f, 0.0f));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            vec3Node->evaluate();
            
            transformNode->evaluate();
            
            lineGraph->evaluate();
        }

        REQUIRE(lineGraph->points.buffer.size() == 5);

        // Change capacity to 3 (note: existing buffer keeps 5 items until next push)
        lineGraph->maxCount = 3;

        // Push one more value, should trigger capacity enforcement
        auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(100.0f, 0.0f, 0.0f));
        graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));
        vec3Node->evaluate();
        
        transformNode->evaluate();
        
        lineGraph->evaluate();

        // Should now have only 3 items
        REQUIRE(lineGraph->points.buffer.size() == 3);
    }
}

// ====================================================================================
// TEST 7: Switch Tracking Axis
// ====================================================================================
TEST_CASE("LineGraphNode switches between tracking axes", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* transformNode = graph.createNode<TransformNode>();
    auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(10.0f, 20.0f, 30.0f));
    auto* lineGraph = graph.createNode<LineGraphNode>();

    graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));
    lineGraph->trackTransform = transformNode;

    SECTION("Switching from X to Y captures different values") {
        // Track X axis
        lineGraph->selectedAxis = LineGraphNode::Axis::X;
        vec3Node->evaluate();
        transformNode->evaluate();
        lineGraph->evaluate();

        REQUIRE_THAT(lineGraph->points[0], WithinAbs(10.0f, 0.001f));

        // Switch to Y axis
        lineGraph->selectedAxis = LineGraphNode::Axis::Y;
        
        transformNode->evaluate();
        
        lineGraph->evaluate();

        REQUIRE_THAT(lineGraph->points[1], WithinAbs(20.0f, 0.001f));
    }

    SECTION("Switching from Y to Z captures different values") {
        // Track Y axis
        lineGraph->selectedAxis = LineGraphNode::Axis::Y;
        vec3Node->evaluate();
        transformNode->evaluate();
        lineGraph->evaluate();

        REQUIRE_THAT(lineGraph->points[0], WithinAbs(20.0f, 0.001f));

        // Switch to Z axis
        lineGraph->selectedAxis = LineGraphNode::Axis::Z;
        
        transformNode->evaluate();
        
        lineGraph->evaluate();

        REQUIRE_THAT(lineGraph->points[1], WithinAbs(30.0f, 0.001f));
    }

    SECTION("All three axes can be tracked sequentially") {
        vec3Node->evaluate();
        transformNode->evaluate();

        // Track X
        lineGraph->selectedAxis = LineGraphNode::Axis::X;
        lineGraph->evaluate();
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(10.0f, 0.001f));

        // Track Y
        lineGraph->selectedAxis = LineGraphNode::Axis::Y;
        
        transformNode->evaluate();
        
        lineGraph->evaluate();
        REQUIRE_THAT(lineGraph->points[1], WithinAbs(20.0f, 0.001f));

        // Track Z
        lineGraph->selectedAxis = LineGraphNode::Axis::Z;
        
        transformNode->evaluate();
        
        lineGraph->evaluate();
        REQUIRE_THAT(lineGraph->points[2], WithinAbs(30.0f, 0.001f));

        REQUIRE(lineGraph->points.buffer.size() == 3);
    }
}

// ====================================================================================
// TEST 8: Switch Between Different TransformNodes
// ====================================================================================
TEST_CASE("LineGraphNode switches between different TransformNodes", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* transform1 = graph.createNode<TransformNode>();
    auto* transform2 = graph.createNode<TransformNode>();
    auto* vec3Node1 = graph.createNode<Vec3ConstantNode>(glm::vec3(5.0f, 5.0f, 5.0f));
    auto* vec3Node2 = graph.createNode<Vec3ConstantNode>(glm::vec3(15.0f, 15.0f, 15.0f));
    auto* lineGraph = graph.createNode<LineGraphNode>();

    graph.connect(vec3Node1->getOutput("Value"), transform1->getInput("Pos"));
    graph.connect(vec3Node2->getOutput("Value"), transform2->getInput("Pos"));

    lineGraph->selectedAxis = LineGraphNode::Axis::X;

    SECTION("Track first TransformNode") {
        lineGraph->trackTransform = transform1;
        vec3Node1->evaluate();
        transform1->evaluate();
        lineGraph->evaluate();

        REQUIRE(lineGraph->points.buffer.size() == 1);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(5.0f, 0.001f));
    }

    SECTION("Switch to second TransformNode") {
        // Track first node
        lineGraph->trackTransform = transform1;
        vec3Node1->evaluate();
        transform1->evaluate();
        lineGraph->evaluate();

        REQUIRE_THAT(lineGraph->points[0], WithinAbs(5.0f, 0.001f));

        // Switch to second node
        lineGraph->trackTransform = transform2;
        vec3Node2->evaluate();
        transform2->evaluate();
        
        lineGraph->evaluate();

        REQUIRE(lineGraph->points.buffer.size() == 2);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(5.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[1], WithinAbs(15.0f, 0.001f));
    }

    SECTION("Multiple switches between nodes") {
        vec3Node1->evaluate();
        vec3Node2->evaluate();
        transform1->evaluate();
        transform2->evaluate();

        // Track transform1
        lineGraph->trackTransform = transform1;
        lineGraph->evaluate();
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(5.0f, 0.001f));

        // Switch to transform2
        lineGraph->trackTransform = transform2;
        
        lineGraph->evaluate();
        REQUIRE_THAT(lineGraph->points[1], WithinAbs(15.0f, 0.001f));

        // Switch back to transform1
        lineGraph->trackTransform = transform1;
        
        lineGraph->evaluate();
        REQUIRE_THAT(lineGraph->points[2], WithinAbs(5.0f, 0.001f));

        REQUIRE(lineGraph->points.buffer.size() == 3);
    }
}

// ====================================================================================
// TEST 9: Disconnect TrackTransform
// ====================================================================================
TEST_CASE("LineGraphNode handles disconnecting trackTransform", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* transformNode = graph.createNode<TransformNode>();
    auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(7.0f, 8.0f, 9.0f));
    auto* lineGraph = graph.createNode<LineGraphNode>();

    graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));
    lineGraph->trackTransform = transformNode;
    lineGraph->selectedAxis = LineGraphNode::Axis::Y;

    SECTION("Track node, then disconnect") {
        // Track the node
        vec3Node->evaluate();
        transformNode->evaluate();
        lineGraph->evaluate();

        REQUIRE(lineGraph->points.buffer.size() == 1);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(8.0f, 0.001f));

        // Disconnect
        lineGraph->trackTransform = nullptr;
        
        lineGraph->evaluate();

        // Buffer size should remain the same (no new points added)
        REQUIRE(lineGraph->points.buffer.size() == 1);
    }

    SECTION("Multiple evaluations after disconnect don't add points") {
        // Track the node
        vec3Node->evaluate();
        transformNode->evaluate();
        lineGraph->evaluate();

        size_t initialSize = lineGraph->points.buffer.size();

        // Disconnect
        lineGraph->trackTransform = nullptr;

        // Multiple evaluations
        for (int i = 0; i < 5; i++) {
            
            lineGraph->evaluate();
        }

        // Size should not change
        REQUIRE(lineGraph->points.buffer.size() == initialSize);
    }

    SECTION("Can reconnect after disconnect") {
        // Track, disconnect, then reconnect
        vec3Node->evaluate();
        transformNode->evaluate();
        lineGraph->evaluate();

        lineGraph->trackTransform = nullptr;
        
        lineGraph->evaluate();

        // Reconnect
        lineGraph->trackTransform = transformNode;
        
        transformNode->evaluate();
        
        lineGraph->evaluate();

        REQUIRE(lineGraph->points.buffer.size() == 2);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(8.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[1], WithinAbs(8.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 10: Multiple Evaluations Accumulate Points
// ====================================================================================
TEST_CASE("LineGraphNode accumulates points over multiple evaluations", "[GraphNodes][LineGraphNode]") {
    NodeGraph graph;
    auto* transformNode = graph.createNode<TransformNode>();
    auto* lineGraph = graph.createNode<LineGraphNode>();

    lineGraph->trackTransform = transformNode;
    lineGraph->selectedAxis = LineGraphNode::Axis::X;
    lineGraph->maxCount = 50;

    SECTION("Accumulate points from changing position") {
        // Simulate movement over time
        for (int frame = 0; frame < 20; frame++) {
            float x = std::sin(static_cast<float>(frame) * 0.1f) * 10.0f;
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(x, 0.0f, 0.0f));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            vec3Node->evaluate();
            
            transformNode->evaluate();
            
            lineGraph->evaluate();
        }

        REQUIRE(lineGraph->points.buffer.size() == 20);

        // Verify first and last points
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(std::sin(0.0f) * 10.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[19], WithinAbs(std::sin(1.9f) * 10.0f, 0.001f));
    }

    SECTION("Points accumulate until capacity is reached") {
        lineGraph->maxCount = 10;

        // Add 15 points
        for (int i = 0; i < 15; i++) {
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(static_cast<float>(i), 0.0f, 0.0f));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            vec3Node->evaluate();
            transformNode->evaluate();
            lineGraph->evaluate();
        }

        // Should only have last 10 points (5-14)
        REQUIRE(lineGraph->points.buffer.size() == 10);
        REQUIRE_THAT(lineGraph->points[0], WithinAbs(5.0f, 0.001f));
        REQUIRE_THAT(lineGraph->points[9], WithinAbs(14.0f, 0.001f));
    }

    SECTION("Accumulate with varying axis selection") {
        // Mix of different axes
        std::vector<LineGraphNode::Axis> axes = {
            LineGraphNode::Axis::X,
            LineGraphNode::Axis::Y,
            LineGraphNode::Axis::Z,
            LineGraphNode::Axis::X,
            LineGraphNode::Axis::Y
        };

        float expectedValues[] = {1.0f, 2.0f, 3.0f, 1.0f, 2.0f};

        for (size_t i = 0; i < axes.size(); i++) {
            auto* vec3Node = graph.createNode<Vec3ConstantNode>(glm::vec3(1.0f, 2.0f, 3.0f));
            graph.connect(vec3Node->getOutput("Value"), transformNode->getInput("Pos"));

            lineGraph->selectedAxis = axes[i];
            vec3Node->evaluate();
            
            transformNode->evaluate();
            
            lineGraph->evaluate();

            REQUIRE_THAT(lineGraph->points[i], WithinAbs(expectedValues[i], 0.001f));
        }

        REQUIRE(lineGraph->points.buffer.size() == 5);
    }
}


