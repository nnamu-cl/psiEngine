#pragma once

#include <volk/volk.h>
#include <string>
#include <vector>

namespace slang { struct IGlobalSession; }
namespace Slang { template<typename T> class ComPtr; }

// Forward declare for blend mode
enum class BlendMode;

// Describes which shaders back a pipeline.
struct PipelineDesc
{
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    BlendMode blendMode;
    bool doubleSided = false;  // If true, disable backface culling
};

class Pipeline
{
public:
    // Creates the VkPipelineLayout + VkPipeline.  setLayouts is the
    // ordered list of descriptor-set layouts that the shaders expect
    // (set 0 = global, set 1 = per-object, etc.).
    bool create(VkDevice device,
                const PipelineDesc& desc,
                const std::vector<VkDescriptorSetLayout>& setLayouts,
                VkFormat colorFormat,
                VkFormat depthFormat);

    void destroy(VkDevice device);

    VkPipeline       pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout layout{   VK_NULL_HANDLE };

private:
    // Compiles GLSL shader to SPIR-V via Slang and wraps it in a VkShaderModule.
    // Shader modules are destroyed immediately after pipeline creation.
    VkShaderModule loadShaderModule(VkDevice device, const std::string& path);

    // Shared Slang global session (created on first use)
    static Slang::ComPtr<slang::IGlobalSession>& getSlangSession();
};
