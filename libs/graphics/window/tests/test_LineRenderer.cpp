//
// LineRenderer Component Tests
// Tests the LineRenderer component functionality
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Components/LineRenderer.h"
#include "Data/LineRendererData.h"
#include "Data/LineProperties.h"

using Catch::Matchers::WithinAbs;

// ====================================================================================
// TEST 1: LineRenderer initialization with data pointer
// ====================================================================================
TEST_CASE("LineRenderer initializes correctly with data pointer", "[LineRenderer][Component]") {
    LineRendererData lineData;

    SECTION("Default construction has null data pointer") {
        LineRenderer renderer;
        REQUIRE(renderer.data == nullptr);
    }

    SECTION("Construction with data pointer sets data correctly") {
        LineRenderer renderer(&lineData);
        REQUIRE(renderer.data == &lineData);
        REQUIRE(renderer.data->points.empty());
        REQUIRE(renderer.data->needsGPUUpdate == false);
    }

    SECTION("Data pointer can be set after construction") {
        LineRenderer renderer;
        renderer.data = &lineData;
        REQUIRE(renderer.data == &lineData);
    }
}

// ====================================================================================
// TEST 2: LineRenderer buildVertexData produces correct output
// ====================================================================================
TEST_CASE("LineRenderer buildVertexData produces correct vertex output", "[LineRenderer][Component]") {
    LineRendererData lineData;
    LineRenderer renderer(&lineData);

    SECTION("Empty data returns empty vertex buffer") {
        auto vertices = renderer.buildVertexData();
        REQUIRE(vertices.empty());
    }

    SECTION("Single point produces single vertex") {
        lineData.points.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        auto vertices = renderer.buildVertexData();

        REQUIRE(vertices.size() == 1);
        REQUIRE(vertices[0].position == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE(vertices[0].color == lineData.properties.color);
        REQUIRE(vertices[0].thickness == lineData.properties.thickness);
        REQUIRE_THAT(vertices[0].distanceAlongLine, WithinAbs(0.0f, 0.001f));
    }

    SECTION("Three points produce correct vertex count and structure") {
        lineData.points.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        lineData.points.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        lineData.points.push_back(glm::vec3(1.0f, 1.0f, 0.0f));

        auto vertices = renderer.buildVertexData();

        REQUIRE(vertices.size() == 3);

        // Check first vertex
        REQUIRE(vertices[0].position == glm::vec3(0.0f, 0.0f, 0.0f));
        REQUIRE_THAT(vertices[0].distanceAlongLine, WithinAbs(0.0f, 0.001f));

        // Check second vertex (distance = 1.0)
        REQUIRE(vertices[1].position == glm::vec3(1.0f, 0.0f, 0.0f));
        REQUIRE_THAT(vertices[1].distanceAlongLine, WithinAbs(1.0f, 0.001f));

        // Check third vertex (distance = 2.0)
        REQUIRE(vertices[2].position == glm::vec3(1.0f, 1.0f, 0.0f));
        REQUIRE_THAT(vertices[2].distanceAlongLine, WithinAbs(2.0f, 0.001f));

        // All vertices should have same color and thickness
        for (const auto& vertex : vertices) {
            REQUIRE(vertex.color == lineData.properties.color);
            REQUIRE(vertex.thickness == lineData.properties.thickness);
        }
    }

    SECTION("Custom color and thickness are applied to all vertices") {
        lineData.points.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        lineData.points.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

        lineData.properties.color = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
        lineData.properties.thickness = 50.0f;

        auto vertices = renderer.buildVertexData();

        REQUIRE(vertices.size() == 2);
        for (const auto& vertex : vertices) {
            REQUIRE(vertex.color == glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));
            REQUIRE(vertex.thickness == 50.0f);
        }
    }

    SECTION("Null data pointer returns empty vertex buffer") {
        LineRenderer rendererNoData;
        auto vertices = rendererNoData.buildVertexData();
        REQUIRE(vertices.empty());
    }
}

// ====================================================================================
// TEST 3: LineRenderer clone preserves data pointer
// ====================================================================================
TEST_CASE("LineRenderer clone preserves data pointer", "[LineRenderer][Component]") {
    LineRendererData lineData;
    lineData.points.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    lineData.points.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

    SECTION("Clone shares the same data pointer") {
        LineRenderer original(&lineData);
        auto cloned = original.clone();

        REQUIRE(cloned != nullptr);

        // Cast to LineRenderer to access data pointer
        auto* clonedRenderer = dynamic_cast<LineRenderer*>(cloned.get());
        REQUIRE(clonedRenderer != nullptr);
        REQUIRE(clonedRenderer->data == original.data);
        REQUIRE(clonedRenderer->data == &lineData);
    }

    SECTION("Cloned renderer produces same vertex output") {
        LineRenderer original(&lineData);
        auto cloned = original.clone();
        auto* clonedRenderer = dynamic_cast<LineRenderer*>(cloned.get());

        auto originalVertices = original.buildVertexData();
        auto clonedVertices = clonedRenderer->buildVertexData();

        REQUIRE(originalVertices.size() == clonedVertices.size());

        for (size_t i = 0; i < originalVertices.size(); ++i) {
            REQUIRE(originalVertices[i].position == clonedVertices[i].position);
            REQUIRE(originalVertices[i].color == clonedVertices[i].color);
            REQUIRE(originalVertices[i].thickness == clonedVertices[i].thickness);
            REQUIRE_THAT(originalVertices[i].distanceAlongLine,
                        WithinAbs(clonedVertices[i].distanceAlongLine, 0.001f));
        }
    }

    SECTION("Clone of renderer with null data has null data") {
        LineRenderer original;
        auto cloned = original.clone();
        auto* clonedRenderer = dynamic_cast<LineRenderer*>(cloned.get());

        REQUIRE(clonedRenderer != nullptr);
        REQUIRE(clonedRenderer->data == nullptr);
    }
}
