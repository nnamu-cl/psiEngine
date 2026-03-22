#pragma once

#include "Layers/Layer.h"
#include "Camera.h"
#include "Scene.h"
#include "PipelineManager.h"
#include "LinePipeline.h"
#include "Resources.h"
#include "Mesh.h"
#include "CameraController.h"
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>

// Forward-declare so we can hold a pointer without pulling in the
// full ApplicationWindow.h (and its SDL / VMA implementation macros).
struct ApplicationWindowData;
struct LineVertex;

// Include LineRendererData (needed for unique_ptr)
#include "Data/LineRendererData.h"

// Tracks GPU buffer info for a single mesh
struct MeshGPUInfo
{
    VkDeviceSize vertexOffset;  // Byte offset in vertex buffer
    VkDeviceSize indexOffset;   // Byte offset in index buffer
    uint32_t     vertexCount;   // Number of vertices
    uint32_t     indexCount;    // Number of indices
};

// Tracks GPU buffer info for a single line
struct LineGPUInfo
{
    VkDeviceSize vertexOffset;  // Byte offset in line buffer
    uint32_t     vertexCount;   // Number of vertices
};

// DefaultGameWorld layer state - grouped for cache locality and clarity
struct DefaultGameWorldData
{
    // Rendering resources (frequently accessed together)
    PipelineManager pipelineManager;
    LinePipeline    linePipeline;
    Resources       resources;

    // Mesh data (frequently accessed together during rendering)
    MeshTable              meshTable;
    VkBuffer               meshBuffer{ VK_NULL_HANDLE };
    VmaAllocation          meshBufferAllocation{ VK_NULL_HANDLE };
    std::vector<MeshGPUInfo> meshGPUInfo;  // Parallel to meshTable indices

    // Line data
    VkBuffer               lineBuffer{ VK_NULL_HANDLE };
    VmaAllocation          lineBufferAllocation{ VK_NULL_HANDLE };
    std::vector<LineGPUInfo> lineGPUInfo;  // Per-object line data
    std::vector<std::unique_ptr<LineRendererData>> lineDataStorage;  // Owned line data

    // Scene data (frequently accessed together)
    Camera camera;
    Scene  scene;

    CameraController cameraController;

    ApplicationWindowData* windowData{ nullptr };


};

class DefaultGameWorld : public Application::Layer
{
public:
    // windowData must outlive this layer -- typically &ApplicationWindow::data.
    explicit DefaultGameWorld(ApplicationWindowData* windowData);

    void OnAttach()  override;
    void OnDetach()  override;
    void OnUpdate(float ts)  override;
    void OnRender(VkCommandBuffer cb, const glm::ivec2& windowSize, uint32_t frameIndex) override;
    void OnEvent(const SDL_Event& e) override;


    // Add a mesh primitive to the scene
    void addMeshPrimitive(const std::string& name, Mesh mesh,
                         const glm::vec3& position = glm::vec3(0.0f),
                         const glm::vec4& color = glm::vec4(1.0f));

    // Add a line primitive to the scene
    void addLinePrimitive(const std::string& name,
                         const std::vector<glm::vec3>& points,
                         const glm::vec4& color = glm::vec4(1.0f));

    // Public data for easy access and testing
    DefaultGameWorldData data;

    ApplicationWindowData* windowData{ nullptr };


private:

    // Helper: Upload all meshes in MeshTable to GPU
    bool uploadMeshesToGPU();


};
