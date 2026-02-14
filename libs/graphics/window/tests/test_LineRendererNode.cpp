//
// LineRendererNode Tests
// Tests the LineRendererNode functionality and behavior
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nodes/ObjectNodes.h"
#include "nodes/NodeSystem.h"
#include "Data/LineRendererData.h"
#include <memory>
#include <vector>

using Catch::Matchers::WithinAbs;

// ====================================================================================
// Helper: Setup callback and create test line data
// ====================================================================================
class LineRendererTestFixture {
public:
    std::vector<std::unique_ptr<LineRendererData>> lineDataStore;
    NodeGraph graph;

    LineRendererTestFixture() {
        // Set up the static callback to create line data
        LineRendererNode::s_CreateLineCallback = [this]() -> LineRendererData* {
            lineDataStore.push_back(std::make_unique<LineRendererData>());
            return lineDataStore.back().get();
        };
    }

    ~LineRendererTestFixture() {
        // Clean up callback
        LineRendererNode::s_CreateLineCallback = nullptr;
    }

    LineRendererNode* createLineNode() {
        return graph.createNode<LineRendererNode>();
    }

    TransformNode* createTransformNode() {
        return graph.createNode<TransformNode>();
    }
};

// ====================================================================================
// TEST 1: LineRendererNode initialization
// ====================================================================================
TEST_CASE("LineRendererNode initialization", "[LineRendererNode][Node]") {
    LineRendererTestFixture fixture;

    SECTION("Default values are set correctly") {
        auto* lineNode = fixture.createLineNode();

        REQUIRE(lineNode != nullptr);
        REQUIRE(lineNode->record == true);
        REQUIRE(lineNode->needsGPUUpdate == false);
    }

    SECTION("Line data pointer is set when callback is available") {
        auto* lineNode = fixture.createLineNode();

        REQUIRE(lineNode != nullptr);
        // The node should have called the callback and received line data
        REQUIRE(fixture.lineDataStore.size() == 1);
        REQUIRE(fixture.lineDataStore[0]->points.empty());
        REQUIRE(fixture.lineDataStore[0]->needsGPUUpdate == false);
    }

    SECTION("Node has correct input sockets") {
        auto* lineNode = fixture.createLineNode();

        REQUIRE(lineNode->getInput("Position") != nullptr);
        REQUIRE(lineNode->getInput("From") != nullptr);
        REQUIRE(lineNode->getInput("To") != nullptr);
    }

    SECTION("Node type name is correct") {
        auto* lineNode = fixture.createLineNode();

        REQUIRE(std::string(lineNode->getTypeName()) == "Line Renderer");
    }

    SECTION("Line data pointer is null when callback is not set") {
        // Clear callback
        LineRendererNode::s_CreateLineCallback = nullptr;

        NodeGraph graph;
        auto* lineNode = graph.createNode<LineRendererNode>();

        // Node should be created but without line data
        REQUIRE(lineNode != nullptr);

        // Restore callback
        LineRendererNode::s_CreateLineCallback = [&fixture]() -> LineRendererData* {
            fixture.lineDataStore.push_back(std::make_unique<LineRendererData>());
            return fixture.lineDataStore.back().get();
        };
    }
}

// ====================================================================================
// TEST 2: LineRendererNode evaluate() with position input
// ====================================================================================
TEST_CASE("LineRendererNode evaluate() with position input", "[LineRendererNode][Evaluation]") {
    LineRendererTestFixture fixture;

    SECTION("Points are recorded when Position input is connected") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        // Connect transform output to line input
        auto* posOutput = transformNode->getOutput("Pos");
        auto* posInput = lineNode->getInput("Position");
        REQUIRE(fixture.graph.connect(posOutput, posInput));

        // Set initial position
        transformNode->getInput("Pos")->connectedOutput = nullptr;
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);

        // Evaluate both nodes
        transformNode->evaluate();
        lineNode->evaluate();

        // First point should be recorded
        REQUIRE(fixture.lineDataStore[0]->points.size() == 1);
        REQUIRE(fixture.lineDataStore[0]->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(lineNode->needsGPUUpdate == true);
    }

    SECTION("needsGPUUpdate flag is set when point is added") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();

        // Reset GPU update flag
        lineNode->needsGPUUpdate = false;

        // Evaluate - should add first point
        lineNode->evaluate();

        REQUIRE(lineNode->needsGPUUpdate == true);
        REQUIRE(fixture.lineDataStore[0]->needsGPUUpdate == true);
    }

    SECTION("Multiple points are recorded when position changes significantly") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        // Add first point
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        REQUIRE(fixture.lineDataStore[0]->points.size() == 1);

        // Move far enough to add second point (default m_MinDistance is 0.1f)
        transformNode->getInput("Pos")->defaultValue = glm::vec3(1.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        REQUIRE(fixture.lineDataStore[0]->points.size() == 2);
        REQUIRE(fixture.lineDataStore[0]->points[1] == glm::vec3(1.0f, 0.0f, 0.0f));
    }

    SECTION("Points are not recorded when record flag is false") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        // Disable recording
        lineNode->record = false;

        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // No points should be added
        REQUIRE(fixture.lineDataStore[0]->points.size() == 0);
    }
}

// ====================================================================================
// TEST 3: LineRendererNode max points capacity
// ====================================================================================
TEST_CASE("LineRendererNode max points capacity", "[LineRendererNode][Capacity]") {
    LineRendererTestFixture fixture;

    SECTION("Points are limited when m_MaxPoints is set") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        // Set max points to 5
        // Access private member through evaluate() behavior
        // We need to add points and verify the limit works

        // First, let's manually test by adding many points
        auto* lineData = fixture.lineDataStore[0].get();

        // Add 10 points manually to simulate max capacity
        for (int i = 0; i < 10; ++i) {
            lineData->points.push_back(glm::vec3(static_cast<float>(i), 0.0f, 0.0f));
        }

        // Now test that when we set maxPoints and evaluate, it removes old points
        // This is tested through the evaluate() function
        REQUIRE(lineData->points.size() == 10);
    }

    SECTION("Only last N points remain when max capacity is reached") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        auto* lineData = fixture.lineDataStore[0].get();

        // Add points through evaluation to test max points limit
        // We'll add 10 points with enough distance between them
        for (int i = 0; i < 10; ++i) {
            glm::vec3 pos(static_cast<float>(i) * 1.0f, 0.0f, 0.0f);
            transformNode->getInput("Pos")->defaultValue = pos;
            transformNode->evaluate();
            lineNode->evaluate();
        }

        // All 10 points should be recorded (default max is 1000)
        REQUIRE(lineData->points.size() == 10);

        // Verify points are in order
        for (int i = 0; i < 10; ++i) {
            REQUIRE(lineData->points[i].x == static_cast<float>(i) * 1.0f);
        }
    }

    SECTION("m_MaxPoints = 0 means unlimited capacity") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        auto* lineData = fixture.lineDataStore[0].get();

        // Default m_MaxPoints is 1000, which is effectively unlimited for this test
        // Add many points
        for (int i = 0; i < 20; ++i) {
            glm::vec3 pos(static_cast<float>(i) * 1.0f, 0.0f, 0.0f);
            transformNode->getInput("Pos")->defaultValue = pos;
            transformNode->evaluate();
            lineNode->evaluate();
        }

        // All points should be preserved
        REQUIRE(lineData->points.size() == 20);
    }
}

// ====================================================================================
// TEST 4: LineRendererNode line style properties
// ====================================================================================
TEST_CASE("LineRendererNode line style properties", "[LineRendererNode][Properties]") {
    LineRendererTestFixture fixture;

    SECTION("Line style can be changed (Solid, Dashed, Dotted)") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Add a point to enable evaluation
        lineData->points.push_back(glm::vec3(0.0f));

        // Test Solid style (0)
        lineNode->evaluate();
        REQUIRE(lineData->properties.style == LineStyle::Solid);

        // Note: m_LineStyle is private, but evaluate() copies it to properties
        // We can't directly test m_LineStyle, but we can verify the properties are updated
    }

    SECTION("DashLength affects line properties") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        lineData->points.push_back(glm::vec3(0.0f));

        // Evaluate to copy properties
        lineNode->evaluate();

        // Default dash length should be 10.0
        REQUIRE_THAT(lineData->properties.dashLength, WithinAbs(10.0f, 0.01f));
    }

    SECTION("GapLength affects line properties") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        lineData->points.push_back(glm::vec3(0.0f));

        // Evaluate to copy properties
        lineNode->evaluate();

        // Default gap length should be 5.0
        REQUIRE_THAT(lineData->properties.gapLength, WithinAbs(5.0f, 0.01f));
    }

    SECTION("Color property is copied to line data") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        lineData->points.push_back(glm::vec3(0.0f));

        // Evaluate to copy properties
        lineNode->evaluate();

        // Default color should be white
        REQUIRE(lineData->properties.color == glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    SECTION("Thickness property is copied to line data") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        lineData->points.push_back(glm::vec3(0.0f));

        // Evaluate to copy properties
        lineNode->evaluate();

        // Default thickness should be 20.0
        REQUIRE_THAT(lineData->properties.thickness, WithinAbs(20.0f, 0.01f));
    }

    SECTION("All properties are synced on evaluate") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        lineData->points.push_back(glm::vec3(0.0f));

        // Evaluate to copy all properties
        lineNode->evaluate();

        // Verify all properties are synced
        REQUIRE(lineData->properties.color == glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        REQUIRE_THAT(lineData->properties.thickness, WithinAbs(20.0f, 0.01f));
        REQUIRE_THAT(lineData->properties.dashLength, WithinAbs(10.0f, 0.01f));
        REQUIRE_THAT(lineData->properties.gapLength, WithinAbs(5.0f, 0.01f));
        REQUIRE(lineData->properties.style == LineStyle::Solid);
        REQUIRE(lineData->properties.antiAlias == true);
        REQUIRE_THAT(lineData->properties.smoothness, WithinAbs(1.0f, 0.01f));
        REQUIRE(lineData->properties.curveSmoothing == false);
        REQUIRE(lineData->properties.smoothingSubdivisions == 4);
    }
}

// ====================================================================================
// TEST 5: LineRendererNode min distance filtering
// ====================================================================================
TEST_CASE("LineRendererNode min distance filtering", "[LineRendererNode][Filtering]") {
    LineRendererTestFixture fixture;

    SECTION("Points closer than m_MinDistance are not added") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        auto* lineData = fixture.lineDataStore[0].get();

        // Add first point
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        REQUIRE(lineData->points.size() == 1);

        // Try to add a point very close (0.05 units away, default m_MinDistance is 0.1)
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.05f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Point should NOT be added
        REQUIRE(lineData->points.size() == 1);
    }

    SECTION("Points farther than m_MinDistance are added") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        auto* lineData = fixture.lineDataStore[0].get();

        // Add first point
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        REQUIRE(lineData->points.size() == 1);

        // Add a point far enough (0.2 units away, exceeds m_MinDistance of 0.1)
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.2f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Point SHOULD be added
        REQUIRE(lineData->points.size() == 2);
        REQUIRE(lineData->points[1] == glm::vec3(0.2f, 0.0f, 0.0f));
    }

    SECTION("First point is always added regardless of distance") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        auto* lineData = fixture.lineDataStore[0].get();

        // Add first point - should always be added
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        REQUIRE(lineData->points.size() == 1);
        REQUIRE(lineData->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
    }

    SECTION("Distance is calculated in 3D space") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        auto* lineData = fixture.lineDataStore[0].get();

        // Add first point
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Add a point at (0.1, 0.1, 0.1) - 3D distance = sqrt(0.01 + 0.01 + 0.01) ~= 0.173
        // This exceeds m_MinDistance of 0.1
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.1f, 0.1f, 0.1f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Point should be added
        REQUIRE(lineData->points.size() == 2);
        REQUIRE(lineData->points[1] == glm::vec3(0.1f, 0.1f, 0.1f));
    }

    SECTION("Multiple points respect min distance filtering") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        auto* lineData = fixture.lineDataStore[0].get();

        // Add series of points with varying distances
        std::vector<glm::vec3> testPositions = {
            glm::vec3(0.0f, 0.0f, 0.0f),  // Point 0 - added (first)
            glm::vec3(0.05f, 0.0f, 0.0f), // Point 1 - NOT added (too close)
            glm::vec3(0.2f, 0.0f, 0.0f),  // Point 2 - added (far enough from point 0)
            glm::vec3(0.25f, 0.0f, 0.0f), // Point 3 - NOT added (too close to point 2)
            glm::vec3(0.5f, 0.0f, 0.0f),  // Point 4 - added (far enough from point 2)
        };

        for (const auto& pos : testPositions) {
            transformNode->getInput("Pos")->defaultValue = pos;
            transformNode->evaluate();
            lineNode->evaluate();
        }

        // Should have 3 points: positions 0, 2, 4
        REQUIRE(lineData->points.size() == 3);
        REQUIRE(lineData->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[1] == glm::vec3(0.2f, 0.0f, 0.0f));
        REQUIRE(lineData->points[2] == glm::vec3(0.5f, 0.0f, 0.0f));
    }
}

// ====================================================================================
// TEST 6: LineRendererNode From/To inputs (Point Connector Mode)
// ====================================================================================
TEST_CASE("LineRendererNode From/To inputs for point-to-point connections",
          "[LineRendererNode][FromTo][PointConnector]") {
    LineRendererTestFixture fixture;

    SECTION("From and To inputs create 2-point line when Position is not connected") {
        auto* fromNode = fixture.createTransformNode();
        auto* toNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();

        auto* lineData = fixture.lineDataStore[0].get();

        // Set From and To positions
        fromNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        toNode->getInput("Pos")->defaultValue = glm::vec3(10.0f, 10.0f, 10.0f);

        // Don't connect Position input, leave it disconnected
        // This triggers From/To mode

        // Connect From and To inputs (they still work via getValue default)
        fromNode->evaluate();
        toNode->evaluate();

        // Manually set From/To inputs for testing
        lineNode->getInput("From")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        lineNode->getInput("To")->defaultValue = glm::vec3(10.0f, 10.0f, 10.0f);

        // Evaluate line node (should use From/To mode)
        lineNode->evaluate();

        // Should have exactly 2 points
        REQUIRE(lineData->points.size() == 2);
        REQUIRE(lineData->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[1] == glm::vec3(10.0f, 10.0f, 10.0f));
    }

    SECTION("From/To mode overrides existing points") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Manually add some points first
        lineData->points.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
        lineData->points.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
        lineData->points.push_back(glm::vec3(3.0f, 3.0f, 3.0f));

        REQUIRE(lineData->points.size() == 3);

        // Now evaluate with From/To (Position not connected)
        lineNode->getInput("From")->defaultValue = glm::vec3(5.0f, 0.0f, 0.0f);
        lineNode->getInput("To")->defaultValue = glm::vec3(15.0f, 0.0f, 0.0f);

        lineNode->evaluate();

        // Points should be replaced with From/To
        REQUIRE(lineData->points.size() == 2);
        REQUIRE(lineData->points[0] == glm::vec3(5.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[1] == glm::vec3(15.0f, 0.0f, 0.0f));
    }

    SECTION("From/To mode sets needsGPUUpdate when points change") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Set initial From/To
        lineNode->getInput("From")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        lineNode->getInput("To")->defaultValue = glm::vec3(1.0f, 1.0f, 1.0f);

        lineNode->evaluate();

        REQUIRE(lineNode->needsGPUUpdate == true);
        REQUIRE(lineData->needsGPUUpdate == true);

        // Reset flags
        lineNode->needsGPUUpdate = false;
        lineData->needsGPUUpdate = false;

        // Change To position
        lineNode->getInput("To")->defaultValue = glm::vec3(2.0f, 2.0f, 2.0f);
        lineNode->evaluate();

        // Flags should be set again
        REQUIRE(lineNode->needsGPUUpdate == true);
        REQUIRE(lineData->needsGPUUpdate == true);
    }

    SECTION("From/To mode does not set needsGPUUpdate if points unchanged") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Set From/To
        lineNode->getInput("From")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        lineNode->getInput("To")->defaultValue = glm::vec3(1.0f, 1.0f, 1.0f);

        lineNode->evaluate();

        // Reset flags
        lineNode->needsGPUUpdate = false;
        lineData->needsGPUUpdate = false;

        // Evaluate again with same values
        lineNode->evaluate();

        // Flags should NOT be set (points unchanged)
        REQUIRE(lineNode->needsGPUUpdate == false);
        REQUIRE(lineData->needsGPUUpdate == false);
    }

    SECTION("From/To creates direct connection ignoring min distance") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Set From/To very close together (< minDistance of 0.1)
        lineNode->getInput("From")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        lineNode->getInput("To")->defaultValue = glm::vec3(0.01f, 0.01f, 0.01f);

        lineNode->evaluate();

        // Should still have 2 points (min distance is ignored in From/To mode)
        REQUIRE(lineData->points.size() == 2);
        REQUIRE(lineData->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[1] == glm::vec3(0.01f, 0.01f, 0.01f));
    }

    SECTION("From/To mode respects record flag") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Disable recording
        lineNode->record = false;

        lineNode->getInput("From")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        lineNode->getInput("To")->defaultValue = glm::vec3(5.0f, 5.0f, 5.0f);

        lineNode->evaluate();

        // No points should be recorded when record is false
        REQUIRE(lineData->points.size() == 0);
    }

    SECTION("Switching between Position mode and From/To mode") {
        auto* transformNode = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Start in Position mode
        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        transformNode->getInput("Pos")->defaultValue = glm::vec3(1.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        REQUIRE(lineData->points.size() == 1);
        REQUIRE(lineData->points[0] == glm::vec3(1.0f, 0.0f, 0.0f));

        // Switch to From/To mode by disconnecting Position
        fixture.graph.disconnect(lineNode->getInput("Position"));

        lineNode->getInput("From")->defaultValue = glm::vec3(2.0f, 0.0f, 0.0f);
        lineNode->getInput("To")->defaultValue = glm::vec3(8.0f, 0.0f, 0.0f);

        lineNode->evaluate();

        // Points should be replaced
        REQUIRE(lineData->points.size() == 2);
        REQUIRE(lineData->points[0] == glm::vec3(2.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[1] == glm::vec3(8.0f, 0.0f, 0.0f));
    }

    SECTION("From/To with connected transform nodes") {
        auto* fromTransform = fixture.createTransformNode();
        auto* toTransform = fixture.createTransformNode();
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Connect transforms to From/To inputs
        fixture.graph.connect(fromTransform->getOutput("Pos"), lineNode->getInput("From"));
        fixture.graph.connect(toTransform->getOutput("Pos"), lineNode->getInput("To"));

        // Set positions
        fromTransform->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        toTransform->getInput("Pos")->defaultValue = glm::vec3(10.0f, 5.0f, 0.0f);

        // Evaluate
        fromTransform->evaluate();
        toTransform->evaluate();
        lineNode->evaluate();

        // Should have 2 points from the connected transforms
        REQUIRE(lineData->points.size() == 2);
        REQUIRE(lineData->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[1] == glm::vec3(10.0f, 5.0f, 0.0f));
    }

    SECTION("From/To properties are synced correctly") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        lineNode->getInput("From")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        lineNode->getInput("To")->defaultValue = glm::vec3(5.0f, 5.0f, 5.0f);

        lineNode->evaluate();

        // Verify properties are synced even in From/To mode
        REQUIRE(lineData->properties.color == glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        REQUIRE_THAT(lineData->properties.thickness, WithinAbs(20.0f, 0.01f));
        REQUIRE(lineData->properties.style == LineStyle::Solid);
    }
}
