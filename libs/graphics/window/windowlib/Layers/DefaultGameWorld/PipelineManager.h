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
            return ((hash<int>()(static_cast<int>(k.shadingMode))
                   ^ (hash<int>()(static_cast<int>(k.blendMode)) << 1)) >> 1)
                   ^ (hash<bool>()(k.doubleSided) << 1);
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
