#pragma once

#include <volk/volk.h>
#include <string>
#include <vector>
#include <glm/vec4.hpp>

#include "ApplicationWindow.h"
#include "LineRendererCompute.h"

namespace slang { struct IGlobalSession; }
namespace Slang { template<typename T> class ComPtr; }

// Forward declare
enum class LineStyle;
struct LineProperties;
struct DefaultGameWorldData;
struct VmaAllocator_T;
typedef VmaAllocator_T* VmaAllocator;

// Describes shaders for line pipeline
struct LinePipelineDesc
{
    std::string vertexShaderPath;
    std::string geometryShaderPath;
    std::string fragmentShaderPath;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    bool depthTest = true;
};


struct LineObjectData
{
    glm::vec4  color;
    float      thickness;
    float      dashLength;
    float      gapLength;
    uint32_t   lineStyle;
    uint32_t   antiAlias;
    float      smoothness;
    uint32_t   objectType;   // 0 = line, 1 = circle
    float      radius;       // used when objectType != 0
    uint32_t   lit;          // 0 = unlit, 1 = lit
    float      _pad0;
    float      _pad1;
    float      _pad2;
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

    void destroy(VkDevice device, VmaAllocator allocator);

    VkPipeline       pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout layout{   VK_NULL_HANDLE };

    // New indirect buffer for gpu driven rendering
    VkBuffer      indirectBuffer{ VK_NULL_HANDLE };
    VmaAllocation indirectBufferAllocation{ VK_NULL_HANDLE };


    //Set for line SSBO
    VkDescriptorSetLayout ssboSetLayout {VK_NULL_HANDLE};
    VkDescriptorPool ssboPool {VK_NULL_HANDLE};
    VkDescriptorSet ssboSet {VK_NULL_HANDLE};

    //Line SSBO itself
    VkBuffer  lineSSBO{VK_NULL_HANDLE};
    VmaAllocation lineSSBOAllocation{VK_NULL_HANDLE};

    // GPU buffer for LineGPUInfo (input to compute shader)
    VkBuffer lineGPUInfoBuffer{VK_NULL_HANDLE};
    VmaAllocation lineGPUInfoBufferAllocation{VK_NULL_HANDLE};

    // Compute pipeline for building indirect draw commands
    LineRendererCompute indirectCompute;
    VkDescriptorPool computeDescPool{VK_NULL_HANDLE};
    VkDescriptorSet computeDescSet{VK_NULL_HANDLE};
    bool computeInitialized{false};

    void DoRender(VkCommandBuffer cb, uint32_t frameIndex, float aspectRatio, DefaultGameWorldData& data);
    bool UploadLinesToGPU(DefaultGameWorldData& data, VmaAllocator allocator);


private:
    // Compiles shader to SPIR-V via Slang and wraps it in a VkShaderModule
    VkShaderModule loadShaderModule(VkDevice device, const std::string& path);

    // Shared Slang global session (reuses from Pipeline)
    static Slang::ComPtr<slang::IGlobalSession>& getSlangSession();

    //Use this for creating the SSBO
    bool createSSBODescriptorInfrastructure(VkDevice device);

    // Initialize the compute pipeline for indirect command generation
    bool initComputePipeline(VkDevice device);

};
