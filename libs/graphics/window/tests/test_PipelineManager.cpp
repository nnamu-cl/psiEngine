//
// PipelineManager Tests
// Tests the PipelineVariantKey structure, equality, and hash function
// CPU-only tests - no Vulkan initialization required
//

#include <catch2/catch_test_macros.hpp>
#include <unordered_set>
#include <unordered_map>

#include "Layers/DefaultGameWorld/PipelineManager.h"

// ====================================================================================
// TEST 1: PipelineVariantKey equality and hashing
// ====================================================================================
TEST_CASE("PipelineVariantKey equality and hashing", "[PipelineManager][PipelineVariantKey]") {

    SECTION("Identical keys are equal") {
        PipelineVariantKey key1{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey key2{ShadingMode::Lit, BlendMode::Opaque, false};

        REQUIRE(key1 == key2);
    }

    SECTION("Keys with different shading modes are not equal") {
        PipelineVariantKey key1{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey key2{ShadingMode::Unlit, BlendMode::Opaque, false};

        REQUIRE_FALSE(key1 == key2);
    }

    SECTION("Keys with different blend modes are not equal") {
        PipelineVariantKey key1{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey key2{ShadingMode::Lit, BlendMode::Transparent, false};

        REQUIRE_FALSE(key1 == key2);
    }

    SECTION("Keys with different doubleSided values are not equal") {
        PipelineVariantKey key1{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey key2{ShadingMode::Lit, BlendMode::Opaque, true};

        REQUIRE_FALSE(key1 == key2);
    }

    SECTION("Hash function produces consistent hashes for identical keys") {
        PipelineVariantKey key1{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey key2{ShadingMode::Lit, BlendMode::Opaque, false};

        std::hash<PipelineVariantKey> hasher;
        REQUIRE(hasher(key1) == hasher(key2));
    }

    SECTION("Hash function produces different hashes for different keys") {
        PipelineVariantKey key1{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey key2{ShadingMode::Unlit, BlendMode::Transparent, true};

        std::hash<PipelineVariantKey> hasher;
        REQUIRE(hasher(key1) != hasher(key2));
    }
}

// ====================================================================================
// TEST 2: PipelineVariantKey distinguishes shading modes and blend modes
// ====================================================================================
TEST_CASE("PipelineVariantKey distinguishes shading modes and blend modes", "[PipelineManager][PipelineVariantKey]") {

    SECTION("Lit and Unlit shading modes create different keys") {
        PipelineVariantKey litKey{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey unlitKey{ShadingMode::Unlit, BlendMode::Opaque, false};

        REQUIRE_FALSE(litKey == unlitKey);

        std::hash<PipelineVariantKey> hasher;
        REQUIRE(hasher(litKey) != hasher(unlitKey));
    }

    SECTION("Different blend modes create different keys") {
        PipelineVariantKey opaqueKey{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey transparentKey{ShadingMode::Lit, BlendMode::Transparent, false};
        PipelineVariantKey additiveKey{ShadingMode::Lit, BlendMode::Additive, false};
        PipelineVariantKey multiplyKey{ShadingMode::Lit, BlendMode::Multiply, false};

        // All keys should be distinct
        REQUIRE_FALSE(opaqueKey == transparentKey);
        REQUIRE_FALSE(opaqueKey == additiveKey);
        REQUIRE_FALSE(opaqueKey == multiplyKey);
        REQUIRE_FALSE(transparentKey == additiveKey);
        REQUIRE_FALSE(transparentKey == multiplyKey);
        REQUIRE_FALSE(additiveKey == multiplyKey);
    }

    SECTION("Blend mode differences produce different hashes") {
        PipelineVariantKey opaqueKey{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey transparentKey{ShadingMode::Lit, BlendMode::Transparent, false};
        PipelineVariantKey additiveKey{ShadingMode::Lit, BlendMode::Additive, false};
        PipelineVariantKey multiplyKey{ShadingMode::Lit, BlendMode::Multiply, false};

        std::hash<PipelineVariantKey> hasher;
        std::unordered_set<size_t> hashes;

        hashes.insert(hasher(opaqueKey));
        hashes.insert(hasher(transparentKey));
        hashes.insert(hasher(additiveKey));
        hashes.insert(hasher(multiplyKey));

        // All 4 hashes should be unique
        REQUIRE(hashes.size() == 4);
    }

    SECTION("Shading mode and blend mode combinations are independent") {
        PipelineVariantKey litOpaque{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey litTransparent{ShadingMode::Lit, BlendMode::Transparent, false};
        PipelineVariantKey unlitOpaque{ShadingMode::Unlit, BlendMode::Opaque, false};
        PipelineVariantKey unlitTransparent{ShadingMode::Unlit, BlendMode::Transparent, false};

        // All combinations should be distinct
        REQUIRE_FALSE(litOpaque == litTransparent);
        REQUIRE_FALSE(litOpaque == unlitOpaque);
        REQUIRE_FALSE(litOpaque == unlitTransparent);
        REQUIRE_FALSE(litTransparent == unlitOpaque);
        REQUIRE_FALSE(litTransparent == unlitTransparent);
        REQUIRE_FALSE(unlitOpaque == unlitTransparent);
    }
}

// ====================================================================================
// TEST 3: PipelineVariantKey with doubleSided flag
// ====================================================================================
TEST_CASE("PipelineVariantKey with doubleSided flag", "[PipelineManager][PipelineVariantKey]") {

    SECTION("doubleSided flag creates distinct keys") {
        PipelineVariantKey singleSided{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey doubleSided{ShadingMode::Lit, BlendMode::Opaque, true};

        REQUIRE_FALSE(singleSided == doubleSided);
    }

    SECTION("doubleSided flag produces different hashes") {
        PipelineVariantKey singleSided{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey doubleSided{ShadingMode::Lit, BlendMode::Opaque, true};

        std::hash<PipelineVariantKey> hasher;
        REQUIRE(hasher(singleSided) != hasher(doubleSided));
    }

    SECTION("doubleSided works with different shading modes") {
        PipelineVariantKey litSingleSided{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey litDoubleSided{ShadingMode::Lit, BlendMode::Opaque, true};
        PipelineVariantKey unlitSingleSided{ShadingMode::Unlit, BlendMode::Opaque, false};
        PipelineVariantKey unlitDoubleSided{ShadingMode::Unlit, BlendMode::Opaque, true};

        // All should be distinct
        REQUIRE_FALSE(litSingleSided == litDoubleSided);
        REQUIRE_FALSE(unlitSingleSided == unlitDoubleSided);
        REQUIRE_FALSE(litSingleSided == unlitSingleSided);
        REQUIRE_FALSE(litDoubleSided == unlitDoubleSided);
    }

    SECTION("doubleSided works with different blend modes") {
        PipelineVariantKey opaqueSingleSided{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey opaqueDoubleSided{ShadingMode::Lit, BlendMode::Opaque, true};
        PipelineVariantKey transparentSingleSided{ShadingMode::Lit, BlendMode::Transparent, false};
        PipelineVariantKey transparentDoubleSided{ShadingMode::Lit, BlendMode::Transparent, true};

        // doubleSided creates distinct keys for each blend mode
        REQUIRE_FALSE(opaqueSingleSided == opaqueDoubleSided);
        REQUIRE_FALSE(transparentSingleSided == transparentDoubleSided);
        REQUIRE_FALSE(opaqueSingleSided == transparentSingleSided);
        REQUIRE_FALSE(opaqueDoubleSided == transparentDoubleSided);
    }
}

// ====================================================================================
// TEST 4: Hash function collision resistance
// ====================================================================================
TEST_CASE("Hash function collision resistance", "[PipelineManager][PipelineVariantKey][Hash]") {

    SECTION("All 16 possible key combinations produce unique hashes") {
        std::hash<PipelineVariantKey> hasher;
        std::unordered_set<size_t> hashes;

        // Test all combinations: 2 shading modes × 4 blend modes × 2 doubleSided = 16 total
        for (auto shadingMode : {ShadingMode::Lit, ShadingMode::Unlit}) {
            for (auto blendMode : {BlendMode::Opaque, BlendMode::Transparent,
                                   BlendMode::Additive, BlendMode::Multiply}) {
                for (auto doubleSided : {false, true}) {
                    PipelineVariantKey key{shadingMode, blendMode, doubleSided};
                    hashes.insert(hasher(key));
                }
            }
        }

        // All 16 hashes should be unique (no collisions)
        REQUIRE(hashes.size() == 16);
    }

    SECTION("Hash function works correctly with unordered_map") {
        std::unordered_map<PipelineVariantKey, int> testMap;

        // Insert all 16 combinations
        int counter = 0;
        for (auto shadingMode : {ShadingMode::Lit, ShadingMode::Unlit}) {
            for (auto blendMode : {BlendMode::Opaque, BlendMode::Transparent,
                                   BlendMode::Additive, BlendMode::Multiply}) {
                for (auto doubleSided : {false, true}) {
                    PipelineVariantKey key{shadingMode, blendMode, doubleSided};
                    testMap[key] = counter++;
                }
            }
        }

        // All 16 entries should be stored
        REQUIRE(testMap.size() == 16);

        // Verify retrieval works correctly
        PipelineVariantKey testKey{ShadingMode::Lit, BlendMode::Transparent, true};
        REQUIRE(testMap.find(testKey) != testMap.end());
    }

    SECTION("Hash distribution is reasonable") {
        std::hash<PipelineVariantKey> hasher;
        std::vector<size_t> hashes;

        // Collect all hashes
        for (auto shadingMode : {ShadingMode::Lit, ShadingMode::Unlit}) {
            for (auto blendMode : {BlendMode::Opaque, BlendMode::Transparent,
                                   BlendMode::Additive, BlendMode::Multiply}) {
                for (auto doubleSided : {false, true}) {
                    PipelineVariantKey key{shadingMode, blendMode, doubleSided};
                    hashes.push_back(hasher(key));
                }
            }
        }

        // Check that hashes are reasonably distributed (no two hashes are identical)
        for (size_t i = 0; i < hashes.size(); ++i) {
            for (size_t j = i + 1; j < hashes.size(); ++j) {
                REQUIRE(hashes[i] != hashes[j]);
            }
        }
    }
}

// ====================================================================================
// TEST 5: PipelineVariantKey with all BlendMode values
// ====================================================================================
TEST_CASE("PipelineVariantKey with all BlendMode values", "[PipelineManager][PipelineVariantKey][BlendMode]") {

    SECTION("All four blend modes create distinct keys") {
        PipelineVariantKey opaqueKey{ShadingMode::Lit, BlendMode::Opaque, false};
        PipelineVariantKey transparentKey{ShadingMode::Lit, BlendMode::Transparent, false};
        PipelineVariantKey additiveKey{ShadingMode::Lit, BlendMode::Additive, false};
        PipelineVariantKey multiplyKey{ShadingMode::Lit, BlendMode::Multiply, false};

        // Verify all are distinct (comprehensive pairwise check)
        REQUIRE_FALSE(opaqueKey == transparentKey);
        REQUIRE_FALSE(opaqueKey == additiveKey);
        REQUIRE_FALSE(opaqueKey == multiplyKey);
        REQUIRE_FALSE(transparentKey == additiveKey);
        REQUIRE_FALSE(transparentKey == multiplyKey);
        REQUIRE_FALSE(additiveKey == multiplyKey);
    }

    SECTION("Blend modes work correctly with Unlit shading") {
        PipelineVariantKey unlitOpaque{ShadingMode::Unlit, BlendMode::Opaque, false};
        PipelineVariantKey unlitTransparent{ShadingMode::Unlit, BlendMode::Transparent, false};
        PipelineVariantKey unlitAdditive{ShadingMode::Unlit, BlendMode::Additive, false};
        PipelineVariantKey unlitMultiply{ShadingMode::Unlit, BlendMode::Multiply, false};

        // All unlit blend mode variants should be distinct
        REQUIRE_FALSE(unlitOpaque == unlitTransparent);
        REQUIRE_FALSE(unlitOpaque == unlitAdditive);
        REQUIRE_FALSE(unlitOpaque == unlitMultiply);
        REQUIRE_FALSE(unlitTransparent == unlitAdditive);
        REQUIRE_FALSE(unlitTransparent == unlitMultiply);
        REQUIRE_FALSE(unlitAdditive == unlitMultiply);
    }

    SECTION("Each blend mode with doubleSided creates unique keys") {
        // Test that each blend mode with doubleSided=true is distinct
        PipelineVariantKey opaqueDouble{ShadingMode::Lit, BlendMode::Opaque, true};
        PipelineVariantKey transparentDouble{ShadingMode::Lit, BlendMode::Transparent, true};
        PipelineVariantKey additiveDouble{ShadingMode::Lit, BlendMode::Additive, true};
        PipelineVariantKey multiplyDouble{ShadingMode::Lit, BlendMode::Multiply, true};

        std::unordered_set<size_t> hashes;
        std::hash<PipelineVariantKey> hasher;

        hashes.insert(hasher(opaqueDouble));
        hashes.insert(hasher(transparentDouble));
        hashes.insert(hasher(additiveDouble));
        hashes.insert(hasher(multiplyDouble));

        REQUIRE(hashes.size() == 4);
    }

    SECTION("Blend modes match the 10 variants in PipelineManager::create()") {
        // The PipelineManager creates 10 variants - verify we can represent all of them
        std::vector<PipelineVariantKey> variants = {
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

        // All 10 variants should be distinct
        std::unordered_set<size_t> hashes;
        std::hash<PipelineVariantKey> hasher;

        for (const auto& variant : variants) {
            hashes.insert(hasher(variant));
        }

        REQUIRE(hashes.size() == 10);
    }
}

// ====================================================================================
// TEST 6: Hash function stability
// ====================================================================================
TEST_CASE("Hash function stability", "[PipelineManager][PipelineVariantKey][Hash]") {

    SECTION("Same key hashed multiple times produces identical results") {
        PipelineVariantKey key{ShadingMode::Lit, BlendMode::Transparent, true};

        std::hash<PipelineVariantKey> hasher;
        size_t hash1 = hasher(key);
        size_t hash2 = hasher(key);
        size_t hash3 = hasher(key);

        REQUIRE(hash1 == hash2);
        REQUIRE(hash2 == hash3);
    }

    SECTION("Recreating identical keys produces identical hashes") {
        std::hash<PipelineVariantKey> hasher;

        PipelineVariantKey key1{ShadingMode::Unlit, BlendMode::Additive, false};
        size_t hash1 = hasher(key1);

        PipelineVariantKey key2{ShadingMode::Unlit, BlendMode::Additive, false};
        size_t hash2 = hasher(key2);

        PipelineVariantKey key3{ShadingMode::Unlit, BlendMode::Additive, false};
        size_t hash3 = hasher(key3);

        REQUIRE(hash1 == hash2);
        REQUIRE(hash2 == hash3);
    }

    SECTION("Hash is deterministic across multiple hasher instances") {
        PipelineVariantKey key{ShadingMode::Lit, BlendMode::Multiply, true};

        std::hash<PipelineVariantKey> hasher1;
        std::hash<PipelineVariantKey> hasher2;
        std::hash<PipelineVariantKey> hasher3;

        size_t hash1 = hasher1(key);
        size_t hash2 = hasher2(key);
        size_t hash3 = hasher3(key);

        REQUIRE(hash1 == hash2);
        REQUIRE(hash2 == hash3);
    }
}

// ====================================================================================
// TEST 7: Edge cases with boundary values
// ====================================================================================
TEST_CASE("Edge cases with boundary values", "[PipelineManager][PipelineVariantKey][EdgeCases]") {

    SECTION("First and last enum values are distinct") {
        // First values
        PipelineVariantKey firstKey{ShadingMode::Lit, BlendMode::Opaque, false};

        // Last values
        PipelineVariantKey lastKey{ShadingMode::Unlit, BlendMode::Multiply, true};

        REQUIRE_FALSE(firstKey == lastKey);

        std::hash<PipelineVariantKey> hasher;
        REQUIRE(hasher(firstKey) != hasher(lastKey));
    }

    SECTION("Consistent doubleSided=false across all combinations") {
        std::unordered_set<size_t> hashes;
        std::hash<PipelineVariantKey> hasher;

        // All combinations with doubleSided=false
        for (auto shadingMode : {ShadingMode::Lit, ShadingMode::Unlit}) {
            for (auto blendMode : {BlendMode::Opaque, BlendMode::Transparent,
                                   BlendMode::Additive, BlendMode::Multiply}) {
                PipelineVariantKey key{shadingMode, blendMode, false};
                hashes.insert(hasher(key));
            }
        }

        // Should have 8 unique hashes (2 shading × 4 blend)
        REQUIRE(hashes.size() == 8);
    }

    SECTION("Consistent doubleSided=true across all combinations") {
        std::unordered_set<size_t> hashes;
        std::hash<PipelineVariantKey> hasher;

        // All combinations with doubleSided=true
        for (auto shadingMode : {ShadingMode::Lit, ShadingMode::Unlit}) {
            for (auto blendMode : {BlendMode::Opaque, BlendMode::Transparent,
                                   BlendMode::Additive, BlendMode::Multiply}) {
                PipelineVariantKey key{shadingMode, blendMode, true};
                hashes.insert(hasher(key));
            }
        }

        // Should have 8 unique hashes (2 shading × 4 blend)
        REQUIRE(hashes.size() == 8);
    }

    SECTION("Mixed configurations are all distinct") {
        std::vector<PipelineVariantKey> keys = {
            {ShadingMode::Lit, BlendMode::Opaque, false},
            {ShadingMode::Lit, BlendMode::Opaque, true},
            {ShadingMode::Lit, BlendMode::Multiply, false},
            {ShadingMode::Lit, BlendMode::Multiply, true},
            {ShadingMode::Unlit, BlendMode::Opaque, false},
            {ShadingMode::Unlit, BlendMode::Opaque, true},
            {ShadingMode::Unlit, BlendMode::Multiply, false},
            {ShadingMode::Unlit, BlendMode::Multiply, true}
        };

        // All keys should be distinct from each other
        for (size_t i = 0; i < keys.size(); ++i) {
            for (size_t j = i + 1; j < keys.size(); ++j) {
                REQUIRE_FALSE(keys[i] == keys[j]);
            }
        }

        // All hashes should be unique
        std::unordered_set<size_t> hashes;
        std::hash<PipelineVariantKey> hasher;
        for (const auto& key : keys) {
            hashes.insert(hasher(key));
        }
        REQUIRE(hashes.size() == keys.size());
    }
}
