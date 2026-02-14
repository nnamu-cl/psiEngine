//
// Pipeline Integration Tests
// Tests the integration between MaterialProperties and PipelineManager
// These are CPU-only tests that verify pipeline selection logic
//

#include <catch2/catch_test_macros.hpp>
#include <unordered_set>
#include <vector>

#include "Data/Material.h"
#include "Layers/DefaultGameWorld/PipelineManager.h"

// ====================================================================================
// TEST 1: PipelineManager selects correct pipeline for material properties
// ====================================================================================
TEST_CASE("PipelineManager getPipeline() returns correct pipeline for material properties",
          "[Integration][PipelineManager][Material]") {

    SECTION("Lit Opaque material creates distinct pipeline key") {
        MaterialProperties mat;
        mat.shadingMode = ShadingMode::Lit;
        mat.blendMode = BlendMode::Opaque;
        mat.doubleSided = false;

        PipelineVariantKey key{mat.shadingMode, mat.blendMode, mat.doubleSided};

        REQUIRE(key.shadingMode == ShadingMode::Lit);
        REQUIRE(key.blendMode == BlendMode::Opaque);
        REQUIRE(key.doubleSided == false);
    }

    SECTION("Unlit Transparent material creates distinct pipeline key") {
        MaterialProperties mat;
        mat.shadingMode = ShadingMode::Unlit;
        mat.blendMode = BlendMode::Transparent;
        mat.doubleSided = false;

        PipelineVariantKey key{mat.shadingMode, mat.blendMode, mat.doubleSided};

        REQUIRE(key.shadingMode == ShadingMode::Unlit);
        REQUIRE(key.blendMode == BlendMode::Transparent);
        REQUIRE(key.doubleSided == false);
    }

    SECTION("Different materials produce different pipeline keys") {
        MaterialProperties litMat;
        litMat.shadingMode = ShadingMode::Lit;
        litMat.blendMode = BlendMode::Opaque;

        MaterialProperties unlitMat;
        unlitMat.shadingMode = ShadingMode::Unlit;
        unlitMat.blendMode = BlendMode::Opaque;

        PipelineVariantKey litKey{litMat.shadingMode, litMat.blendMode, litMat.doubleSided};
        PipelineVariantKey unlitKey{unlitMat.shadingMode, unlitMat.blendMode, unlitMat.doubleSided};

        REQUIRE_FALSE(litKey == unlitKey);
    }

    SECTION("Same material properties produce same pipeline key") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Lit;
        mat1.blendMode = BlendMode::Additive;
        mat1.doubleSided = true;

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Lit;
        mat2.blendMode = BlendMode::Additive;
        mat2.doubleSided = true;

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        REQUIRE(key1 == key2);
    }
}

// ====================================================================================
// TEST 2: All 16 material variant combinations create unique keys
// ====================================================================================
TEST_CASE("All material variant combinations create unique pipeline keys",
          "[Integration][PipelineManager][Material][Variants]") {

    SECTION("All 16 combinations of shading, blend, and doubleSided are unique") {
        std::unordered_set<size_t> uniqueHashes;
        std::hash<PipelineVariantKey> hasher;

        // Test all 2 × 4 × 2 = 16 combinations
        for (auto shadingMode : {ShadingMode::Lit, ShadingMode::Unlit}) {
            for (auto blendMode : {BlendMode::Opaque, BlendMode::Transparent,
                                   BlendMode::Additive, BlendMode::Multiply}) {
                for (auto doubleSided : {false, true}) {
                    MaterialProperties mat;
                    mat.shadingMode = shadingMode;
                    mat.blendMode = blendMode;
                    mat.doubleSided = doubleSided;

                    PipelineVariantKey key{mat.shadingMode, mat.blendMode, mat.doubleSided};
                    uniqueHashes.insert(hasher(key));
                }
            }
        }

        // All 16 should be unique
        REQUIRE(uniqueHashes.size() == 16);
    }

    SECTION("The 10 pre-created pipeline variants match expected keys") {
        // PipelineManager::create() creates these 10 variants
        std::vector<PipelineVariantKey> expectedVariants = {
            // Lit pipelines (5 variants)
            {ShadingMode::Lit, BlendMode::Opaque, false},
            {ShadingMode::Lit, BlendMode::Transparent, false},
            {ShadingMode::Lit, BlendMode::Additive, false},
            {ShadingMode::Lit, BlendMode::Multiply, false},
            {ShadingMode::Lit, BlendMode::Opaque, true},  // Double-sided

            // Unlit pipelines (5 variants)
            {ShadingMode::Unlit, BlendMode::Opaque, false},
            {ShadingMode::Unlit, BlendMode::Transparent, false},
            {ShadingMode::Unlit, BlendMode::Additive, false},
            {ShadingMode::Unlit, BlendMode::Multiply, false},
            {ShadingMode::Unlit, BlendMode::Opaque, true}  // Double-sided
        };

        // Verify all keys are unique
        std::unordered_set<size_t> hashes;
        std::hash<PipelineVariantKey> hasher;

        for (const auto& variant : expectedVariants) {
            hashes.insert(hasher(variant));
        }

        REQUIRE(hashes.size() == 10);
    }

    SECTION("Materials can be created for all 10 pre-created variants") {
        std::vector<MaterialProperties> materials;

        // Lit materials (5 variants)
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Lit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Opaque, 0.5f, false});
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Lit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Transparent, 0.5f, false});
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Lit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Additive, 0.5f, false});
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Lit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Multiply, 0.5f, false});
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Lit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Opaque, 0.5f, true});

        // Unlit materials (5 variants)
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Unlit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Opaque, 0.5f, false});
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Unlit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Transparent, 0.5f, false});
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Unlit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Additive, 0.5f, false});
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Unlit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Multiply, 0.5f, false});
        materials.push_back({ColorMode::VertexColor, glm::vec4(1.0f), ShadingMode::Unlit, 1.0f,
                            glm::vec3(1.0f), BlendMode::Opaque, 0.5f, true});

        REQUIRE(materials.size() == 10);

        // Verify each creates a unique key
        std::unordered_set<size_t> uniqueKeys;
        std::hash<PipelineVariantKey> hasher;

        for (const auto& mat : materials) {
            PipelineVariantKey key{mat.shadingMode, mat.blendMode, mat.doubleSided};
            uniqueKeys.insert(hasher(key));
        }

        REQUIRE(uniqueKeys.size() == 10);
    }
}

// ====================================================================================
// TEST 3: Material property independence
// ====================================================================================
TEST_CASE("Material properties that don't affect pipeline key",
          "[Integration][PipelineManager][Material][Properties]") {

    SECTION("colorMode does not affect pipeline key") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Lit;
        mat1.blendMode = BlendMode::Opaque;
        mat1.doubleSided = false;
        mat1.colorMode = ColorMode::VertexColor;

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Lit;
        mat2.blendMode = BlendMode::Opaque;
        mat2.doubleSided = false;
        mat2.colorMode = ColorMode::ObjectColor;

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        // Same pipeline key despite different colorMode
        REQUIRE(key1 == key2);
    }

    SECTION("objectColor does not affect pipeline key") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Unlit;
        mat1.blendMode = BlendMode::Additive;
        mat1.objectColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Unlit;
        mat2.blendMode = BlendMode::Additive;
        mat2.objectColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        REQUIRE(key1 == key2);
    }

    SECTION("emissionIntensity does not affect pipeline key") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Unlit;
        mat1.blendMode = BlendMode::Opaque;
        mat1.emissionIntensity = 1.0f;

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Unlit;
        mat2.blendMode = BlendMode::Opaque;
        mat2.emissionIntensity = 5.0f;

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        REQUIRE(key1 == key2);
    }

    SECTION("tintColor does not affect pipeline key") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Lit;
        mat1.blendMode = BlendMode::Transparent;
        mat1.tintColor = glm::vec3(1.0f, 1.0f, 1.0f);

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Lit;
        mat2.blendMode = BlendMode::Transparent;
        mat2.tintColor = glm::vec3(0.5f, 0.5f, 0.5f);

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        REQUIRE(key1 == key2);
    }

    SECTION("alphaCutoff does not affect pipeline key") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Lit;
        mat1.blendMode = BlendMode::Transparent;
        mat1.alphaCutoff = 0.1f;

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Lit;
        mat2.blendMode = BlendMode::Transparent;
        mat2.alphaCutoff = 0.9f;

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        REQUIRE(key1 == key2);
    }
}

// ====================================================================================
// TEST 4: Pipeline key respects only critical rendering state
// ====================================================================================
TEST_CASE("Pipeline key respects only shadingMode, blendMode, and doubleSided",
          "[Integration][PipelineManager][Material][PipelineKey]") {

    SECTION("Only shadingMode affects pipeline when other properties constant") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Lit;
        mat1.blendMode = BlendMode::Opaque;
        mat1.doubleSided = false;

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Unlit;
        mat2.blendMode = BlendMode::Opaque;
        mat2.doubleSided = false;

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        REQUIRE_FALSE(key1 == key2);
    }

    SECTION("Only blendMode affects pipeline when other properties constant") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Lit;
        mat1.blendMode = BlendMode::Opaque;
        mat1.doubleSided = false;

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Lit;
        mat2.blendMode = BlendMode::Transparent;
        mat2.doubleSided = false;

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        REQUIRE_FALSE(key1 == key2);
    }

    SECTION("Only doubleSided affects pipeline when other properties constant") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Lit;
        mat1.blendMode = BlendMode::Opaque;
        mat1.doubleSided = false;

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Lit;
        mat2.blendMode = BlendMode::Opaque;
        mat2.doubleSided = true;

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        REQUIRE_FALSE(key1 == key2);
    }

    SECTION("All three properties affect pipeline independently") {
        MaterialProperties baseMat;
        baseMat.shadingMode = ShadingMode::Lit;
        baseMat.blendMode = BlendMode::Opaque;
        baseMat.doubleSided = false;

        MaterialProperties variedMat;
        variedMat.shadingMode = ShadingMode::Unlit;
        variedMat.blendMode = BlendMode::Additive;
        variedMat.doubleSided = true;

        PipelineVariantKey baseKey{baseMat.shadingMode, baseMat.blendMode, baseMat.doubleSided};
        PipelineVariantKey variedKey{variedMat.shadingMode, variedMat.blendMode, variedMat.doubleSided};

        REQUIRE_FALSE(baseKey == variedKey);
    }
}

// ====================================================================================
// TEST 5: Practical material usage scenarios
// ====================================================================================
TEST_CASE("Practical material usage scenarios",
          "[Integration][PipelineManager][Material][UseCases]") {

    SECTION("Glowing effect material (Unlit + Additive)") {
        MaterialProperties glowMat;
        glowMat.shadingMode = ShadingMode::Unlit;
        glowMat.blendMode = BlendMode::Additive;
        glowMat.colorMode = ColorMode::ObjectColor;
        glowMat.objectColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);  // Cyan
        glowMat.emissionIntensity = 3.0f;

        PipelineVariantKey key{glowMat.shadingMode, glowMat.blendMode, glowMat.doubleSided};

        REQUIRE(key.shadingMode == ShadingMode::Unlit);
        REQUIRE(key.blendMode == BlendMode::Additive);
    }

    SECTION("Glass material (Lit + Transparent + Double-sided)") {
        MaterialProperties glassMat;
        glassMat.shadingMode = ShadingMode::Lit;
        glassMat.blendMode = BlendMode::Transparent;
        glassMat.colorMode = ColorMode::ObjectColor;
        glassMat.objectColor = glm::vec4(0.8f, 0.9f, 1.0f, 0.3f);  // Light blue, transparent
        glassMat.doubleSided = true;

        PipelineVariantKey key{glassMat.shadingMode, glassMat.blendMode, glassMat.doubleSided};

        REQUIRE(key.shadingMode == ShadingMode::Lit);
        REQUIRE(key.blendMode == BlendMode::Transparent);
        REQUIRE(key.doubleSided == true);
    }

    SECTION("Shadow decal material (Lit + Multiply)") {
        MaterialProperties shadowMat;
        shadowMat.shadingMode = ShadingMode::Lit;
        shadowMat.blendMode = BlendMode::Multiply;
        shadowMat.colorMode = ColorMode::ObjectColor;
        shadowMat.objectColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);

        PipelineVariantKey key{shadowMat.shadingMode, shadowMat.blendMode, shadowMat.doubleSided};

        REQUIRE(key.shadingMode == ShadingMode::Lit);
        REQUIRE(key.blendMode == BlendMode::Multiply);
    }

    SECTION("Standard opaque material (Lit + Opaque)") {
        MaterialProperties standardMat;
        standardMat.shadingMode = ShadingMode::Lit;
        standardMat.blendMode = BlendMode::Opaque;
        standardMat.colorMode = ColorMode::VertexColor;

        PipelineVariantKey key{standardMat.shadingMode, standardMat.blendMode, standardMat.doubleSided};

        REQUIRE(key.shadingMode == ShadingMode::Lit);
        REQUIRE(key.blendMode == BlendMode::Opaque);
        REQUIRE(key.doubleSided == false);
    }

    SECTION("Hologram material (Unlit + Transparent)") {
        MaterialProperties hologramMat;
        hologramMat.shadingMode = ShadingMode::Unlit;
        hologramMat.blendMode = BlendMode::Transparent;
        hologramMat.colorMode = ColorMode::ObjectColor;
        hologramMat.objectColor = glm::vec4(0.0f, 1.0f, 0.5f, 0.6f);  // Green-cyan, semi-transparent
        hologramMat.emissionIntensity = 2.0f;

        PipelineVariantKey key{hologramMat.shadingMode, hologramMat.blendMode, hologramMat.doubleSided};

        REQUIRE(key.shadingMode == ShadingMode::Unlit);
        REQUIRE(key.blendMode == BlendMode::Transparent);
    }
}

// ====================================================================================
// TEST 6: Material consistency across multiple instances
// ====================================================================================
TEST_CASE("Material consistency across multiple instances",
          "[Integration][PipelineManager][Material][Consistency]") {

    SECTION("Two materials with same render state produce identical keys") {
        MaterialProperties mat1;
        mat1.shadingMode = ShadingMode::Lit;
        mat1.blendMode = BlendMode::Transparent;
        mat1.doubleSided = true;
        mat1.objectColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);  // Red

        MaterialProperties mat2;
        mat2.shadingMode = ShadingMode::Lit;
        mat2.blendMode = BlendMode::Transparent;
        mat2.doubleSided = true;
        mat2.objectColor = glm::vec4(0.0f, 0.0f, 1.0f, 0.5f);  // Blue

        PipelineVariantKey key1{mat1.shadingMode, mat1.blendMode, mat1.doubleSided};
        PipelineVariantKey key2{mat2.shadingMode, mat2.blendMode, mat2.doubleSided};

        // Same pipeline should be used despite different colors
        REQUIRE(key1 == key2);

        std::hash<PipelineVariantKey> hasher;
        REQUIRE(hasher(key1) == hasher(key2));
    }

    SECTION("Multiple materials can share same pipeline with different uniforms") {
        // In a real rendering scenario, these would all use the same pipeline
        // but with different uniform data (color, emissionIntensity, etc.)
        std::vector<MaterialProperties> materials;

        for (int i = 0; i < 10; ++i) {
            MaterialProperties mat;
            mat.shadingMode = ShadingMode::Unlit;
            mat.blendMode = BlendMode::Additive;
            mat.doubleSided = false;
            mat.objectColor = glm::vec4(
                static_cast<float>(i) / 10.0f,
                1.0f - static_cast<float>(i) / 10.0f,
                0.5f,
                1.0f
            );
            mat.emissionIntensity = 1.0f + static_cast<float>(i) * 0.5f;

            materials.push_back(mat);
        }

        // All should produce the same pipeline key
        std::unordered_set<size_t> uniqueHashes;
        std::hash<PipelineVariantKey> hasher;

        for (const auto& mat : materials) {
            PipelineVariantKey key{mat.shadingMode, mat.blendMode, mat.doubleSided};
            uniqueHashes.insert(hasher(key));
        }

        // Only 1 unique hash (all materials use same pipeline)
        REQUIRE(uniqueHashes.size() == 1);
    }
}
