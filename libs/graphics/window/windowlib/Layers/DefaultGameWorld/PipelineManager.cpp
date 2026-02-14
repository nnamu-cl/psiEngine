#include "PipelineManager.h"
#include <iostream>

bool PipelineManager::create(VkDevice device,
                             const std::vector<VkDescriptorSetLayout>& setLayouts,
                             VkFormat colorFormat,
                             VkFormat depthFormat)
{
    // Create all commonly used pipeline variants
    std::vector<PipelineVariantKey> variants = {
        // Lit pipelines
        { ShadingMode::Lit, BlendMode::Opaque, false },
        { ShadingMode::Lit, BlendMode::Transparent, false },
        { ShadingMode::Lit, BlendMode::Additive, false },
        { ShadingMode::Lit, BlendMode::Multiply, false },
        { ShadingMode::Lit, BlendMode::Opaque, true },  // Double-sided

        // Unlit pipelines
        { ShadingMode::Unlit, BlendMode::Opaque, false },
        { ShadingMode::Unlit, BlendMode::Transparent, false },
        { ShadingMode::Unlit, BlendMode::Additive, false },
        { ShadingMode::Unlit, BlendMode::Multiply, false },
        { ShadingMode::Unlit, BlendMode::Opaque, true }  // Double-sided
    };

    for (const auto& variant : variants)
    {
        if (!createPipelineVariant(device, variant, setLayouts, colorFormat, depthFormat))
        {
            std::cerr << "Failed to create pipeline variant\n";
            return false;
        }
    }

    std::cout << "Created " << pipelines.size() << " pipeline variants\n";
    return true;
}

bool PipelineManager::createPipelineVariant(VkDevice device,
                                            const PipelineVariantKey& key,
                                            const std::vector<VkDescriptorSetLayout>& setLayouts,
                                            VkFormat colorFormat,
                                            VkFormat depthFormat)
{
    // Configure pipeline descriptor
    PipelineDesc desc;
    desc.blendMode = key.blendMode;
    desc.doubleSided = key.doubleSided;

    // Determine shader paths based on shading mode
    if (key.shadingMode == ShadingMode::Lit)
    {
        desc.vertexShaderPath = "libs/graphics/window/assets/shaders/DefaultGameWorld/default.vert.slang";
        desc.fragmentShaderPath = "libs/graphics/window/assets/shaders/DefaultGameWorld/default.frag.slang";
    }
    else  // Unlit
    {
        desc.vertexShaderPath = "libs/graphics/window/assets/shaders/DefaultGameWorld/unlit.vert.slang";
        desc.fragmentShaderPath = "libs/graphics/window/assets/shaders/DefaultGameWorld/unlit.frag.slang";
    }

    // Create the pipeline
    Pipeline pipeline;
    if (!pipeline.create(device, desc, setLayouts, colorFormat, depthFormat))
    {
        return false;
    }

    // Store the pipeline
    pipelines[key] = std::move(pipeline);
    return true;
}

Pipeline* PipelineManager::getPipeline(const MaterialProperties& material)
{
    PipelineVariantKey key{
        material.shadingMode,
        material.blendMode,
        material.doubleSided
    };

    auto it = pipelines.find(key);
    if (it != pipelines.end())
    {
        return &it->second;
    }

    std::cerr << "Pipeline variant not found for requested material properties\n";
    return nullptr;
}

void PipelineManager::destroy(VkDevice device)
{
    for (auto& [key, pipeline] : pipelines)
    {
        pipeline.destroy(device);
    }
    pipelines.clear();
}
