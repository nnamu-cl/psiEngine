//
// Material System Tests
// Tests the MaterialProperties structure and enum values
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Data/Material.h"

using Catch::Matchers::WithinAbs;

// ====================================================================================
// TEST 1: MaterialProperties default initialization
// ====================================================================================
TEST_CASE("MaterialProperties default initialization", "[Material][MaterialProperties]") {
    MaterialProperties mat;

    SECTION("Default color mode is VertexColor") {
        REQUIRE(mat.colorMode == ColorMode::VertexColor);
    }

    SECTION("Default object color is white opaque") {
        REQUIRE(mat.objectColor == glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    SECTION("Default shading mode is Lit") {
        REQUIRE(mat.shadingMode == ShadingMode::Lit);
    }

    SECTION("Default emission intensity is 1.0") {
        REQUIRE_THAT(mat.emissionIntensity, WithinAbs(1.0f, 0.001f));
    }

    SECTION("Default tint color is white") {
        REQUIRE(mat.tintColor == glm::vec3(1.0f, 1.0f, 1.0f));
    }

    SECTION("Default blend mode is Opaque") {
        REQUIRE(mat.blendMode == BlendMode::Opaque);
    }

    SECTION("Default alpha cutoff is 0.5") {
        REQUIRE_THAT(mat.alphaCutoff, WithinAbs(0.5f, 0.001f));
    }

    SECTION("Default double sided is false") {
        REQUIRE(mat.doubleSided == false);
    }
}

// ====================================================================================
// TEST 2: ColorMode enum values
// ====================================================================================
TEST_CASE("ColorMode enum values", "[Material][ColorMode]") {
    MaterialProperties mat;

    SECTION("VertexColor mode uses per-vertex colors") {
        mat.colorMode = ColorMode::VertexColor;
        REQUIRE(mat.colorMode == ColorMode::VertexColor);
    }

    SECTION("ObjectColor mode uses single color") {
        mat.colorMode = ColorMode::ObjectColor;
        REQUIRE(mat.colorMode == ColorMode::ObjectColor);
    }

    SECTION("objectColor is used when mode is ObjectColor") {
        mat.colorMode = ColorMode::ObjectColor;
        mat.objectColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.8f);

        REQUIRE(mat.colorMode == ColorMode::ObjectColor);
        REQUIRE(mat.objectColor == glm::vec4(1.0f, 0.0f, 0.0f, 0.8f));
    }

    SECTION("objectColor can be any RGBA value") {
        mat.colorMode = ColorMode::ObjectColor;
        mat.objectColor = glm::vec4(0.2f, 0.5f, 0.9f, 0.3f);

        REQUIRE(mat.objectColor.r == 0.2f);
        REQUIRE(mat.objectColor.g == 0.5f);
        REQUIRE(mat.objectColor.b == 0.9f);
        REQUIRE(mat.objectColor.a == 0.3f);
    }

    SECTION("ColorMode can be switched at runtime") {
        mat.colorMode = ColorMode::VertexColor;
        REQUIRE(mat.colorMode == ColorMode::VertexColor);

        mat.colorMode = ColorMode::ObjectColor;
        REQUIRE(mat.colorMode == ColorMode::ObjectColor);

        mat.colorMode = ColorMode::VertexColor;
        REQUIRE(mat.colorMode == ColorMode::VertexColor);
    }
}

// ====================================================================================
// TEST 3: ShadingMode toggles between Lit and Unlit
// ====================================================================================
TEST_CASE("ShadingMode toggles between Lit and Unlit", "[Material][ShadingMode]") {
    MaterialProperties mat;

    SECTION("Set to Unlit: emissionIntensity is relevant") {
        mat.shadingMode = ShadingMode::Unlit;
        mat.emissionIntensity = 2.0f;

        REQUIRE(mat.shadingMode == ShadingMode::Unlit);
        REQUIRE_THAT(mat.emissionIntensity, WithinAbs(2.0f, 0.001f));
    }

    SECTION("Set to Lit: lighting calculations apply") {
        mat.shadingMode = ShadingMode::Lit;

        REQUIRE(mat.shadingMode == ShadingMode::Lit);
    }

    SECTION("emissionIntensity can be increased for brighter output") {
        mat.shadingMode = ShadingMode::Unlit;
        mat.emissionIntensity = 5.0f;

        REQUIRE(mat.shadingMode == ShadingMode::Unlit);
        REQUIRE_THAT(mat.emissionIntensity, WithinAbs(5.0f, 0.001f));
    }

    SECTION("emissionIntensity at 1.0 is normal brightness") {
        mat.shadingMode = ShadingMode::Unlit;
        mat.emissionIntensity = 1.0f;

        REQUIRE_THAT(mat.emissionIntensity, WithinAbs(1.0f, 0.001f));
    }

    SECTION("ShadingMode can toggle at runtime") {
        mat.shadingMode = ShadingMode::Lit;
        REQUIRE(mat.shadingMode == ShadingMode::Lit);

        mat.shadingMode = ShadingMode::Unlit;
        REQUIRE(mat.shadingMode == ShadingMode::Unlit);

        mat.shadingMode = ShadingMode::Lit;
        REQUIRE(mat.shadingMode == ShadingMode::Lit);
    }

    SECTION("emissionIntensity can be fractional for dimming") {
        mat.shadingMode = ShadingMode::Unlit;
        mat.emissionIntensity = 0.5f;

        REQUIRE_THAT(mat.emissionIntensity, WithinAbs(0.5f, 0.001f));
    }
}

// ====================================================================================
// TEST 4: BlendMode affects rendering properties
// ====================================================================================
TEST_CASE("BlendMode affects rendering properties", "[Material][BlendMode]") {
    MaterialProperties mat;

    SECTION("Opaque mode: fully opaque, no blending") {
        mat.blendMode = BlendMode::Opaque;
        REQUIRE(mat.blendMode == BlendMode::Opaque);
    }

    SECTION("Transparent mode: alpha blending") {
        mat.blendMode = BlendMode::Transparent;
        REQUIRE(mat.blendMode == BlendMode::Transparent);
    }

    SECTION("Additive mode: additive blending for glowing effects") {
        mat.blendMode = BlendMode::Additive;
        REQUIRE(mat.blendMode == BlendMode::Additive);
    }

    SECTION("Multiply mode: multiplicative blending for shadows") {
        mat.blendMode = BlendMode::Multiply;
        REQUIRE(mat.blendMode == BlendMode::Multiply);
    }

    SECTION("All blend modes are distinct enum values") {
        REQUIRE(BlendMode::Opaque != BlendMode::Transparent);
        REQUIRE(BlendMode::Opaque != BlendMode::Additive);
        REQUIRE(BlendMode::Opaque != BlendMode::Multiply);
        REQUIRE(BlendMode::Transparent != BlendMode::Additive);
        REQUIRE(BlendMode::Transparent != BlendMode::Multiply);
        REQUIRE(BlendMode::Additive != BlendMode::Multiply);
    }

    SECTION("BlendMode can be switched at runtime") {
        mat.blendMode = BlendMode::Opaque;
        REQUIRE(mat.blendMode == BlendMode::Opaque);

        mat.blendMode = BlendMode::Transparent;
        REQUIRE(mat.blendMode == BlendMode::Transparent);

        mat.blendMode = BlendMode::Additive;
        REQUIRE(mat.blendMode == BlendMode::Additive);

        mat.blendMode = BlendMode::Multiply;
        REQUIRE(mat.blendMode == BlendMode::Multiply);
    }

    SECTION("Transparent mode can be used with objectColor alpha") {
        mat.blendMode = BlendMode::Transparent;
        mat.colorMode = ColorMode::ObjectColor;
        mat.objectColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);

        REQUIRE(mat.blendMode == BlendMode::Transparent);
        REQUIRE(mat.objectColor.a == 0.5f);
    }
}
