#pragma once

#include "Pipeline.h"
#include "../../Data/Material.h"
#include <volk/volk.h>
#include <unordered_map>
#include <string>

// Key for identifying pipeline variants
struct PipelineVariantKey
{
    ShadingMode shadingMode;
    BlendMode blendMode;
    bool doubleSided;

    bool operator==(const PipelineVariantKey& other) const
    {
        return shadingMode == other.shadingMode &&
               blendMode == other.blendMode &&
               doubleSided == other.doubleSided;
    }
};

// Hash function for PipelineVariantKey
namespace std {
    template<>
    struct hash<PipelineVariantKey>
    {
        size_t operator()(const PipelineVariantKey& k) const
        {
            // Use boost-style hash_combine to properly combine hash values
            size_t seed = 0;

            // Combine shadingMode hash
            seed ^= hash<int>()(static_cast<int>(k.shadingMode)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            // Combine blendMode hash
            seed ^= hash<int>()(static_cast<int>(k.blendMode)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            // Combine doubleSided hash
            seed ^= hash<bool>()(k.doubleSided) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
}

// Manages multiple pipeline variants for different rendering modes
class PipelineManager
{
public:
    // Creates all pipeline variants
    bool create(VkDevice device,
                const std::vector<VkDescriptorSetLayout>& setLayouts,
                VkFormat colorFormat,
                VkFormat depthFormat);

    void destroy(VkDevice device);

    // Get pipeline for specific material properties
    Pipeline* getPipeline(const MaterialProperties& material);

private:
    // Pipeline storage
    std::unordered_map<PipelineVariantKey, Pipeline> pipelines;

    // Helper: Create a single pipeline variant
    bool createPipelineVariant(VkDevice device,
                              const PipelineVariantKey& key,
                              const std::vector<VkDescriptorSetLayout>& setLayouts,
                              VkFormat colorFormat,
                              VkFormat depthFormat);
};
