#pragma once

#include "Layers/Layer.h"
#include "Camera.h"
#include "Scene.h"
#include "Pipeline.h"
#include "Resources.h"
#include "Mesh.h"
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>

// Forward-declare so we can hold a pointer without pulling in the
// full ApplicationWindow.h (and its SDL / VMA implementation macros).
struct ApplicationWindowData;

// Tracks GPU buffer info for a single mesh
struct MeshGPUInfo
{
    VkDeviceSize vertexOffset;  // Byte offset in vertex buffer
    VkDeviceSize indexOffset;   // Byte offset in index buffer
    uint32_t     vertexCount;   // Number of vertices
    uint32_t     indexCount;    // Number of indices
};

// DefaultGameWorld layer state - grouped for cache locality and clarity
struct DefaultGameWorldData
{
    // Rendering resources (frequently accessed together)
    Pipeline  pipeline;
    Resources resources;

    // Mesh data (frequently accessed together during rendering)
    MeshTable              meshTable;
    VkBuffer               meshBuffer{ VK_NULL_HANDLE };
    VmaAllocation          meshBufferAllocation{ VK_NULL_HANDLE };
    std::vector<MeshGPUInfo> meshGPUInfo;  // Parallel to meshTable indices

    // Scene data (frequently accessed together)
    Camera camera;
    Scene  scene;
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
    void OnUIRender() override;

    // Public data for easy access and testing
    DefaultGameWorldData data;

private:
    ApplicationWindowData* m_WindowData{ nullptr };

    // Helper: Upload all meshes in MeshTable to GPU
    bool uploadMeshesToGPU();
};
