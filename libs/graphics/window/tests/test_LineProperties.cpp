//
// Unit tests for LineProperties and LineRendererData structures.
// Tests default initialization, enum behavior, and GPU update flag handling.
//

#include <catch2/catch_test_macros.hpp>

#include "Data/LineProperties.h"
#include "Data/LineRendererData.h"
#include <glm/glm.hpp>

// ---------------------------------------------------------------------------
// LineProperties Default Initialization
// ---------------------------------------------------------------------------
TEST_CASE("LineProperties default initialization - color")
{
    LineProperties props;

    // Default color should be white (1, 1, 1, 1)
    REQUIRE(props.color.r == 1.0f);
    REQUIRE(props.color.g == 1.0f);
    REQUIRE(props.color.b == 1.0f);
    REQUIRE(props.color.a == 1.0f);
}

TEST_CASE("LineProperties default initialization - thickness")
{
    LineProperties props;

    // Default thickness should be 20.0f
    REQUIRE(props.thickness == 20.0f);
}

TEST_CASE("LineProperties default initialization - style")
{
    LineProperties props;

    // Default style should be Solid
    REQUIRE(props.style == LineStyle::Solid);
}

TEST_CASE("LineProperties default initialization - antiAlias")
{
    LineProperties props;

    // Default antiAlias should be true
    REQUIRE(props.antiAlias == true);
}

TEST_CASE("LineProperties default initialization - smoothness")
{
    LineProperties props;

    // Default smoothness should be 1.0f
    REQUIRE(props.smoothness == 1.0f);
}

TEST_CASE("LineProperties default initialization - all properties")
{
    LineProperties props;

    // Verify all defaults in one test
    REQUIRE(props.color == glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    REQUIRE(props.thickness == 20.0f);
    REQUIRE(props.style == LineStyle::Solid);
    REQUIRE(props.antiAlias == true);
    REQUIRE(props.smoothness == 1.0f);
    REQUIRE(props.topology == LineTopology::LineStrip);
    REQUIRE(props.depthTest == true);
}

// ---------------------------------------------------------------------------
// LineStyle Enum Behavior
// ---------------------------------------------------------------------------
TEST_CASE("LineStyle enum - Solid value")
{
    LineProperties props;
    props.style = LineStyle::Solid;

    REQUIRE(props.style == LineStyle::Solid);
    REQUIRE(props.style != LineStyle::Dashed);
    REQUIRE(props.style != LineStyle::Dotted);
}

TEST_CASE("LineStyle enum - Dashed value")
{
    LineProperties props;
    props.style = LineStyle::Dashed;

    REQUIRE(props.style == LineStyle::Dashed);
    REQUIRE(props.style != LineStyle::Solid);
    REQUIRE(props.style != LineStyle::Dotted);
}

TEST_CASE("LineStyle enum - Dotted value")
{
    LineProperties props;
    props.style = LineStyle::Dotted;

    REQUIRE(props.style == LineStyle::Dotted);
    REQUIRE(props.style != LineStyle::Solid);
    REQUIRE(props.style != LineStyle::Dashed);
}

TEST_CASE("LineStyle enum - can be assigned and compared")
{
    LineProperties props1;
    LineProperties props2;

    props1.style = LineStyle::Solid;
    props2.style = LineStyle::Solid;

    REQUIRE(props1.style == props2.style);

    props2.style = LineStyle::Dashed;
    REQUIRE(props1.style != props2.style);
}

// ---------------------------------------------------------------------------
// LineTopology Enum Behavior
// ---------------------------------------------------------------------------
TEST_CASE("LineTopology enum - LineList value")
{
    LineProperties props;
    props.topology = LineTopology::LineList;

    REQUIRE(props.topology == LineTopology::LineList);
    REQUIRE(props.topology != LineTopology::LineStrip);
    REQUIRE(props.topology != LineTopology::LineLoop);
}

TEST_CASE("LineTopology enum - LineStrip value")
{
    LineProperties props;
    props.topology = LineTopology::LineStrip;

    REQUIRE(props.topology == LineTopology::LineStrip);
    REQUIRE(props.topology != LineTopology::LineList);
    REQUIRE(props.topology != LineTopology::LineLoop);
}

TEST_CASE("LineTopology enum - LineLoop value")
{
    LineProperties props;
    props.topology = LineTopology::LineLoop;

    REQUIRE(props.topology == LineTopology::LineLoop);
    REQUIRE(props.topology != LineTopology::LineList);
    REQUIRE(props.topology != LineTopology::LineStrip);
}

TEST_CASE("LineTopology enum - default is LineStrip")
{
    LineProperties props;

    REQUIRE(props.topology == LineTopology::LineStrip);
}

// ---------------------------------------------------------------------------
// LineRendererData needsGPUUpdate Flag
// ---------------------------------------------------------------------------
TEST_CASE("LineRendererData needsGPUUpdate - starts false")
{
    LineRendererData data;

    // needsGPUUpdate should default to false
    REQUIRE(data.needsGPUUpdate == false);
}

TEST_CASE("LineRendererData needsGPUUpdate - can be set to true")
{
    LineRendererData data;

    // Modify points and set flag
    data.points.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    data.points.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
    data.needsGPUUpdate = true;

    REQUIRE(data.needsGPUUpdate == true);
}

TEST_CASE("LineRendererData needsGPUUpdate - flag persists after modification")
{
    LineRendererData data;

    // Add initial points
    data.points.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    data.needsGPUUpdate = true;

    REQUIRE(data.needsGPUUpdate == true);

    // Add more points - flag should still be true
    data.points.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    data.points.push_back(glm::vec3(1.0f, 1.0f, 0.0f));

    REQUIRE(data.needsGPUUpdate == true);
}

TEST_CASE("LineRendererData needsGPUUpdate - can be reset to false")
{
    LineRendererData data;

    // Set to true
    data.needsGPUUpdate = true;
    REQUIRE(data.needsGPUUpdate == true);

    // Reset to false (simulating GPU update completed)
    data.needsGPUUpdate = false;
    REQUIRE(data.needsGPUUpdate == false);
}

TEST_CASE("LineRendererData needsGPUUpdate - mutable flag can be modified on const object")
{
    const LineRendererData data;

    // Even though data is const, needsGPUUpdate is mutable
    data.needsGPUUpdate = true;
    REQUIRE(data.needsGPUUpdate == true);

    data.needsGPUUpdate = false;
    REQUIRE(data.needsGPUUpdate == false);
}

// ---------------------------------------------------------------------------
// LineRendererData Structure Tests
// ---------------------------------------------------------------------------
TEST_CASE("LineRendererData default initialization - empty points")
{
    LineRendererData data;

    REQUIRE(data.points.empty());
    REQUIRE(data.points.size() == 0);
}

TEST_CASE("LineRendererData default initialization - properties initialized")
{
    LineRendererData data;

    // Properties should be default-initialized
    REQUIRE(data.properties.color == glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    REQUIRE(data.properties.thickness == 20.0f);
    REQUIRE(data.properties.style == LineStyle::Solid);
}

TEST_CASE("LineRendererData can store points")
{
    LineRendererData data;

    data.points.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    data.points.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    data.points.push_back(glm::vec3(1.0f, 1.0f, 0.0f));

    REQUIRE(data.points.size() == 3);
    REQUIRE(data.points[0] == glm::vec3(0.0f, 0.0f, 0.0f));
    REQUIRE(data.points[1] == glm::vec3(1.0f, 0.0f, 0.0f));
    REQUIRE(data.points[2] == glm::vec3(1.0f, 1.0f, 0.0f));
}

TEST_CASE("LineRendererData properties can be modified")
{
    LineRendererData data;

    // Modify properties
    data.properties.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    data.properties.thickness = 5.0f;
    data.properties.style = LineStyle::Dashed;

    REQUIRE(data.properties.color == glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    REQUIRE(data.properties.thickness == 5.0f);
    REQUIRE(data.properties.style == LineStyle::Dashed);
}

// ---------------------------------------------------------------------------
// Additional LineCapStyle and LineJoinStyle Tests
// ---------------------------------------------------------------------------
TEST_CASE("LineCapStyle enum - all values accessible")
{
    LineProperties props;

    props.capStyle = LineCapStyle::Butt;
    REQUIRE(props.capStyle == LineCapStyle::Butt);

    props.capStyle = LineCapStyle::Round;
    REQUIRE(props.capStyle == LineCapStyle::Round);

    props.capStyle = LineCapStyle::Square;
    REQUIRE(props.capStyle == LineCapStyle::Square);
}

TEST_CASE("LineJoinStyle enum - all values accessible")
{
    LineProperties props;

    props.joinStyle = LineJoinStyle::Miter;
    REQUIRE(props.joinStyle == LineJoinStyle::Miter);

    props.joinStyle = LineJoinStyle::Round;
    REQUIRE(props.joinStyle == LineJoinStyle::Round);

    props.joinStyle = LineJoinStyle::Bevel;
    REQUIRE(props.joinStyle == LineJoinStyle::Bevel);
}

TEST_CASE("LineProperties BlendMode enum - all values accessible")
{
    LineProperties props;

    props.blendMode = LineProperties::LineBlendMode::Opaque;
    REQUIRE(props.blendMode == LineProperties::LineBlendMode::Opaque);

    props.blendMode = LineProperties::LineBlendMode::Transparent;
    REQUIRE(props.blendMode == LineProperties::LineBlendMode::Transparent);

    props.blendMode = LineProperties::LineBlendMode::Additive;
    REQUIRE(props.blendMode == LineProperties::LineBlendMode::Additive);
}
