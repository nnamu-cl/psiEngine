#pragma once

#include <volk/volk.h>
#include <string>
#include <vector>

namespace slang { struct IGlobalSession; }
namespace Slang { template<typename T> class ComPtr; }

// Forward declare
enum class LineStyle;
struct LineProperties;

// Describes shaders for line pipeline
struct LinePipelineDesc
{
    std::string vertexShaderPath;
    std::string geometryShaderPath;
    std::string fragmentShaderPath;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    bool depthTest = true;
};

class LinePipeline
{
public:
    // Creates the VkPipelineLayout + VkPipeline for line rendering
    bool create(VkDevice device,
                const LinePipelineDesc& desc,
                const std::vector<VkDescriptorSetLayout>& setLayouts,
                VkFormat colorFormat,
                VkFormat depthFormat);

    void destroy(VkDevice device);

    VkPipeline       pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout layout{   VK_NULL_HANDLE };

private:
    // Compiles shader to SPIR-V via Slang and wraps it in a VkShaderModule
    VkShaderModule loadShaderModule(VkDevice device, const std::string& path);

    // Shared Slang global session (reuses from Pipeline)
    static Slang::ComPtr<slang::IGlobalSession>& getSlangSession();
};
