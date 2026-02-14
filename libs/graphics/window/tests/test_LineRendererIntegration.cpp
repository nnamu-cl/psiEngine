//
// LineRenderer Integration Tests
// Tests the integration between LineRendererNode, LineRenderer component,
// Material system, and multiple transform tracking
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nodes/ObjectNodes.h"
#include "nodes/NodeSystem.h"
#include "Components/LineRenderer.h"
#include "Data/LineRendererData.h"
#include "Data/Material.h"

using Catch::Matchers::WithinAbs;

// ====================================================================================
// Test Fixture for Integration Tests
// ====================================================================================
class IntegrationTestFixture {
public:
    std::vector<std::unique_ptr<LineRendererData>> lineDataStore;
    NodeGraph graph;

    IntegrationTestFixture() {
        // Set up the static callback to create line data
        LineRendererNode::s_CreateLineCallback = [this]() -> LineRendererData* {
            lineDataStore.push_back(std::make_unique<LineRendererData>());
            return lineDataStore.back().get();
        };
    }

    ~IntegrationTestFixture() {
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
// TEST 1: LineRendererNode + LineRenderer component workflow
// ====================================================================================
TEST_CASE("LineRendererNode and LineRenderer component integration workflow",
          "[Integration][LineRenderer][LineRendererNode]") {
    IntegrationTestFixture fixture;

    SECTION("Node and component share same LineRendererData") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Create LineRenderer component with shared data
        LineRenderer component(lineData);

        REQUIRE(component.data == lineData);
        REQUIRE(lineData->points.empty());

        // Add points through the node
        auto* transformNode = fixture.createTransformNode();
        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        transformNode->getInput("Pos")->defaultValue = glm::vec3(1.0f, 2.0f, 3.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Verify component can see the data added by node
        REQUIRE(component.data->points.size() == 1);
        REQUIRE(component.data->points[0] == glm::vec3(1.0f, 2.0f, 3.0f));
    }

    SECTION("Component can build vertex data from node-updated data") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        LineRenderer component(lineData);

        // Add multiple points through node
        auto* transformNode = fixture.createTransformNode();
        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        std::vector<glm::vec3> positions = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f)
        };

        for (const auto& pos : positions) {
            transformNode->getInput("Pos")->defaultValue = pos;
            transformNode->evaluate();
            lineNode->evaluate();
        }

        // Component should be able to build vertex data
        auto vertices = component.buildVertexData();

        REQUIRE(vertices.size() == 3);
        REQUIRE(vertices[0].position == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(vertices[1].position == glm::vec3(1.0f, 0.0f, 0.0f));
        REQUIRE(vertices[2].position == glm::vec3(1.0f, 1.0f, 0.0f));

        // Verify distance along line is calculated correctly
        REQUIRE_THAT(vertices[0].distanceAlongLine, WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(vertices[1].distanceAlongLine, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(vertices[2].distanceAlongLine, WithinAbs(2.0f, 0.001f));
    }

    SECTION("needsGPUUpdate flag propagates from node to data") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        LineRenderer component(lineData);

        // Initially no GPU update needed
        REQUIRE(lineData->needsGPUUpdate == false);
        REQUIRE(lineNode->needsGPUUpdate == false);

        // Add point through node
        auto* transformNode = fixture.createTransformNode();
        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Both flags should be set
        REQUIRE(lineNode->needsGPUUpdate == true);
        REQUIRE(lineData->needsGPUUpdate == true);
        REQUIRE(component.data->needsGPUUpdate == true);
    }

    SECTION("Component reflects node property changes") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        LineRenderer component(lineData);

        // Add a point
        auto* transformNode = fixture.createTransformNode();
        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Check that properties were synced from node to data
        REQUIRE(lineData->properties.color == glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        REQUIRE_THAT(lineData->properties.thickness, WithinAbs(20.0f, 0.01f));
        REQUIRE(lineData->properties.style == LineStyle::Solid);

        // Component should build vertices with these properties
        auto vertices = component.buildVertexData();
        REQUIRE(vertices[0].color == glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        REQUIRE(vertices[0].thickness == 20.0f);
    }

    SECTION("Multiple components can share same data pointer") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        LineRenderer component1(lineData);
        LineRenderer component2(lineData);

        // Add points through node
        auto* transformNode = fixture.createTransformNode();
        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        transformNode->getInput("Pos")->defaultValue = glm::vec3(5.0f, 5.0f, 5.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Both components see the same data
        REQUIRE(component1.data->points.size() == 1);
        REQUIRE(component2.data->points.size() == 1);
        REQUIRE(component1.data->points[0] == glm::vec3(5.0f, 5.0f, 5.0f));
        REQUIRE(component2.data->points[0] == glm::vec3(5.0f, 5.0f, 5.0f));
    }

    SECTION("Cloned component shares same data pointer") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        LineRenderer original(lineData);

        // Add points
        lineData->points.push_back(glm::vec3(1.0f, 2.0f, 3.0f));
        lineData->points.push_back(glm::vec3(4.0f, 5.0f, 6.0f));

        // Clone component
        auto cloned = original.clone();
        auto* clonedRenderer = dynamic_cast<LineRenderer*>(cloned.get());

        REQUIRE(clonedRenderer != nullptr);
        REQUIRE(clonedRenderer->data == original.data);
        REQUIRE(clonedRenderer->data == lineData);

        // Both see same points
        auto originalVertices = original.buildVertexData();
        auto clonedVertices = clonedRenderer->buildVertexData();

        REQUIRE(originalVertices.size() == clonedVertices.size());
        REQUIRE(originalVertices.size() == 2);
    }
}

// ====================================================================================
// TEST 2: Material system with multiple shading modes
// ====================================================================================
TEST_CASE("Material system with multiple shading modes",
          "[Integration][Material][ColorMode][ShadingMode]") {

    SECTION("ObjectColor mode uses objectColor field") {
        MaterialProperties mat;
        mat.colorMode = ColorMode::ObjectColor;
        mat.objectColor = glm::vec4(0.8f, 0.2f, 0.1f, 0.9f);

        REQUIRE(mat.colorMode == ColorMode::ObjectColor);
        REQUIRE(mat.objectColor == glm::vec4(0.8f, 0.2f, 0.1f, 0.9f));

        // In rendering, this color would be used uniformly across the object
        // (Verified by shader code inspection - not testable in CPU tests)
    }

    SECTION("VertexColor mode with tintColor multiplies vertex colors") {
        MaterialProperties mat;
        mat.colorMode = ColorMode::VertexColor;
        mat.tintColor = glm::vec3(0.5f, 1.0f, 0.5f);  // Green tint

        REQUIRE(mat.colorMode == ColorMode::VertexColor);
        REQUIRE(mat.tintColor == glm::vec3(0.5f, 1.0f, 0.5f));

        // In rendering, vertex colors would be multiplied by tintColor
        // Example: white vertex (1,1,1) * (0.5, 1.0, 0.5) = (0.5, 1.0, 0.5)
    }

    SECTION("Lit vs Unlit shading modes have different properties") {
        MaterialProperties litMat;
        litMat.shadingMode = ShadingMode::Lit;

        MaterialProperties unlitMat;
        unlitMat.shadingMode = ShadingMode::Unlit;
        unlitMat.emissionIntensity = 2.5f;

        REQUIRE(litMat.shadingMode == ShadingMode::Lit);
        REQUIRE(unlitMat.shadingMode == ShadingMode::Unlit);

        // Lit materials use lighting calculations
        // Unlit materials use emissionIntensity as brightness multiplier
        REQUIRE_THAT(unlitMat.emissionIntensity, WithinAbs(2.5f, 0.001f));
    }

    SECTION("Material with Unlit and ObjectColor creates uniform emissive color") {
        MaterialProperties mat;
        mat.shadingMode = ShadingMode::Unlit;
        mat.colorMode = ColorMode::ObjectColor;
        mat.objectColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);  // Orange
        mat.emissionIntensity = 3.0f;

        REQUIRE(mat.shadingMode == ShadingMode::Unlit);
        REQUIRE(mat.colorMode == ColorMode::ObjectColor);
        REQUIRE(mat.objectColor == glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        REQUIRE_THAT(mat.emissionIntensity, WithinAbs(3.0f, 0.001f));

        // In shader: finalColor = objectColor.rgb * emissionIntensity
        // Result would be bright orange: (1.0, 0.5, 0.0) * 3.0 = (3.0, 1.5, 0.0)
    }

    SECTION("BlendMode affects how colors are composited") {
        MaterialProperties opaqueMat;
        opaqueMat.blendMode = BlendMode::Opaque;

        MaterialProperties transparentMat;
        transparentMat.blendMode = BlendMode::Transparent;
        transparentMat.objectColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);

        MaterialProperties additiveMat;
        additiveMat.blendMode = BlendMode::Additive;
        additiveMat.shadingMode = ShadingMode::Unlit;
        additiveMat.emissionIntensity = 5.0f;

        REQUIRE(opaqueMat.blendMode == BlendMode::Opaque);
        REQUIRE(transparentMat.blendMode == BlendMode::Transparent);
        REQUIRE(additiveMat.blendMode == BlendMode::Additive);

        // Opaque: No blending, draws over background
        // Transparent: Alpha blending, mixes with background based on alpha
        // Additive: Adds color to background, good for glowing effects
    }

    SECTION("All material property combinations are valid") {
        // Test a complex material configuration
        MaterialProperties complexMat;
        complexMat.shadingMode = ShadingMode::Unlit;
        complexMat.colorMode = ColorMode::VertexColor;
        complexMat.tintColor = glm::vec3(1.0f, 0.8f, 0.6f);
        complexMat.emissionIntensity = 2.0f;
        complexMat.blendMode = BlendMode::Additive;
        complexMat.doubleSided = true;

        REQUIRE(complexMat.shadingMode == ShadingMode::Unlit);
        REQUIRE(complexMat.colorMode == ColorMode::VertexColor);
        REQUIRE(complexMat.tintColor == glm::vec3(1.0f, 0.8f, 0.6f));
        REQUIRE_THAT(complexMat.emissionIntensity, WithinAbs(2.0f, 0.001f));
        REQUIRE(complexMat.blendMode == BlendMode::Additive);
        REQUIRE(complexMat.doubleSided == true);
    }

    SECTION("DoubleSided flag affects rendering behavior") {
        MaterialProperties singleSided;
        singleSided.doubleSided = false;

        MaterialProperties doubleSided;
        doubleSided.doubleSided = true;

        REQUIRE(singleSided.doubleSided == false);
        REQUIRE(doubleSided.doubleSided == true);

        // Single-sided: Back faces are culled (not rendered)
        // Double-sided: Both front and back faces are rendered
    }
}

// ====================================================================================
// TEST 3: LineRendererNode tracks multiple transform positions
// ====================================================================================
TEST_CASE("LineRendererNode tracks multiple transform positions",
          "[Integration][LineRendererNode][Transform][Tracking]") {
    IntegrationTestFixture fixture;

    SECTION("Single LineRendererNode tracks three transform positions in sequence") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();

        // Create 3 transform nodes at different positions
        auto* transform1 = fixture.createTransformNode();
        auto* transform2 = fixture.createTransformNode();
        auto* transform3 = fixture.createTransformNode();

        transform1->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transform2->getInput("Pos")->defaultValue = glm::vec3(5.0f, 0.0f, 0.0f);
        transform3->getInput("Pos")->defaultValue = glm::vec3(5.0f, 5.0f, 0.0f);

        // Connect first transform to line node
        fixture.graph.connect(transform1->getOutput("Pos"), lineNode->getInput("Position"));

        // Evaluate first position
        transform1->evaluate();
        lineNode->evaluate();

        REQUIRE(lineData->points.size() == 1);
        REQUIRE(lineData->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));

        // Disconnect first, connect second
        fixture.graph.disconnect(lineNode->getInput("Position"));
        fixture.graph.connect(transform2->getOutput("Pos"), lineNode->getInput("Position"));

        // Evaluate second position
        transform2->evaluate();
        lineNode->evaluate();

        REQUIRE(lineData->points.size() == 2);
        REQUIRE(lineData->points[1] == glm::vec3(5.0f, 0.0f, 0.0f));

        // Disconnect second, connect third
        fixture.graph.disconnect(lineNode->getInput("Position"));
        fixture.graph.connect(transform3->getOutput("Pos"), lineNode->getInput("Position"));

        // Evaluate third position
        transform3->evaluate();
        lineNode->evaluate();

        REQUIRE(lineData->points.size() == 3);
        REQUIRE(lineData->points[2] == glm::vec3(5.0f, 5.0f, 0.0f));

        // Verify all points are in correct order
        REQUIRE(lineData->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[1] == glm::vec3(5.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[2] == glm::vec3(5.0f, 5.0f, 0.0f));
    }

    SECTION("Line passes through all three points in correct order") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        auto* transformNode = fixture.createTransformNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        // Define three waypoints
        std::vector<glm::vec3> waypoints = {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        };

        // Add each waypoint
        for (const auto& waypoint : waypoints) {
            transformNode->getInput("Pos")->defaultValue = waypoint;
            transformNode->evaluate();
            lineNode->evaluate();
        }

        // Verify line has all three points
        REQUIRE(lineData->points.size() == 3);

        // Verify order
        for (size_t i = 0; i < waypoints.size(); ++i) {
            REQUIRE(lineData->points[i] == waypoints[i]);
        }

        // Create LineRenderer component to build the line
        LineRenderer renderer(lineData);
        auto vertices = renderer.buildVertexData();

        REQUIRE(vertices.size() == 3);

        // Verify vertices match the waypoints
        REQUIRE(vertices[0].position == glm::vec3(1.0f, 0.0f, 0.0f));
        REQUIRE(vertices[1].position == glm::vec3(1.0f, 1.0f, 0.0f));
        REQUIRE(vertices[2].position == glm::vec3(0.0f, 1.0f, 0.0f));

        // Verify distances along line
        REQUIRE_THAT(vertices[0].distanceAlongLine, WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(vertices[1].distanceAlongLine, WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(vertices[2].distanceAlongLine, WithinAbs(2.0f, 0.001f));
    }

    SECTION("Distance filtering works when tracking multiple positions") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        auto* transformNode = fixture.createTransformNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        // Default m_MinDistance is 0.1f
        // Add positions with varying distances
        std::vector<glm::vec3> positions = {
            glm::vec3(0.0f, 0.0f, 0.0f),    // Point 0 - added (first)
            glm::vec3(0.05f, 0.0f, 0.0f),   // Point 1 - NOT added (too close, < 0.1)
            glm::vec3(0.15f, 0.0f, 0.0f),   // Point 2 - added (0.15 from point 0)
            glm::vec3(0.20f, 0.0f, 0.0f),   // Point 3 - NOT added (0.05 from point 2)
            glm::vec3(0.30f, 0.0f, 0.0f)    // Point 4 - added (0.15 from point 2)
        };

        for (const auto& pos : positions) {
            transformNode->getInput("Pos")->defaultValue = pos;
            transformNode->evaluate();
            lineNode->evaluate();
        }

        // Should have 3 points: 0, 2, 4
        REQUIRE(lineData->points.size() == 3);
        REQUIRE(lineData->points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(lineData->points[1] == glm::vec3(0.15f, 0.0f, 0.0f));
        REQUIRE(lineData->points[2] == glm::vec3(0.30f, 0.0f, 0.0f));
    }

    SECTION("Multiple transforms at same location result in single point") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        auto* transformNode = fixture.createTransformNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        // Set same position multiple times
        glm::vec3 samePos(5.0f, 5.0f, 5.0f);

        for (int i = 0; i < 5; ++i) {
            transformNode->getInput("Pos")->defaultValue = samePos;
            transformNode->evaluate();
            lineNode->evaluate();
        }

        // Only first point should be added (others filtered by min distance)
        REQUIRE(lineData->points.size() == 1);
        REQUIRE(lineData->points[0] == samePos);
    }

    SECTION("Line tracks moving transform over time") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        auto* transformNode = fixture.createTransformNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        // Simulate movement along a path
        std::vector<glm::vec3> path;
        for (int i = 0; i <= 10; ++i) {
            float t = static_cast<float>(i) / 10.0f;
            glm::vec3 pos(t * 10.0f, std::sin(t * 3.14159f) * 2.0f, 0.0f);
            path.push_back(pos);

            transformNode->getInput("Pos")->defaultValue = pos;
            transformNode->evaluate();
            lineNode->evaluate();
        }

        // Should have recorded all 11 points (distances are > 0.1)
        REQUIRE(lineData->points.size() == 11);

        // Verify first and last points
        REQUIRE(lineData->points[0] == path[0]);
        REQUIRE(lineData->points[10] == path[10]);

        // Verify GPU update flag was set
        REQUIRE(lineData->needsGPUUpdate == true);
    }

    SECTION("needsGPUUpdate is set when tracking positions") {
        auto* lineNode = fixture.createLineNode();
        auto* lineData = fixture.lineDataStore[0].get();
        auto* transformNode = fixture.createTransformNode();

        fixture.graph.connect(transformNode->getOutput("Pos"), lineNode->getInput("Position"));

        // Initial state
        REQUIRE(lineData->needsGPUUpdate == false);

        // Add first position
        transformNode->getInput("Pos")->defaultValue = glm::vec3(0.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        REQUIRE(lineData->needsGPUUpdate == true);

        // Reset flag
        lineData->needsGPUUpdate = false;

        // Add second position (far enough to be added)
        transformNode->getInput("Pos")->defaultValue = glm::vec3(5.0f, 0.0f, 0.0f);
        transformNode->evaluate();
        lineNode->evaluate();

        // Flag should be set again
        REQUIRE(lineData->needsGPUUpdate == true);
    }
}
