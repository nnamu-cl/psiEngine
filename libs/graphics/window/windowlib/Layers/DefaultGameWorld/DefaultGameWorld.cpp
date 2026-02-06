#include "DefaultGameWorld.h"
#include "ApplicationWindow.h"   // full definition of ApplicationWindowData
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"
#include <iostream>
#include <cstring>

#include "imgui.h"

DefaultGameWorld::DefaultGameWorld(ApplicationWindowData* windowData)
    : m_WindowData{ windowData }
{}

void DefaultGameWorld::OnAttach()
{
    // Create resources (descriptor layouts, pool, uniform buffers)
    if (!data.resources.create(m_WindowData->device,
                                m_WindowData->allocator,
                                maxFramesInFlight))
    {
        std::cerr << "Failed to create DefaultGameWorld resources\n";
        return;
    }

    // Create pipeline with default shaders
    PipelineDesc desc{
        .vertexShaderPath   = "libs/graphics/window/shaders/DefaultGameWorld/default.vert.slang",
        .fragmentShaderPath = "libs/graphics/window/shaders/DefaultGameWorld/default.frag.slang"
    };

    std::vector<VkDescriptorSetLayout> setLayouts = { data.resources.globalSetLayout };
    if (!data.pipeline.create(m_WindowData->device,
                              desc,
                              setLayouts,
                              m_WindowData->swapchainImageFormat,
                              m_WindowData->depthFormat))
    {
        std::cerr << "Failed to create DefaultGameWorld pipeline\n";
        return;
    }

    // Scene starts empty - meshes can be added via UI
    std::cout << "DefaultGameWorld layer attached successfully\n";
    std::cout << "  Scene ready - use UI to add objects\n";
}

bool DefaultGameWorld::uploadMeshesToGPU()
{
    uint32_t meshCount = data.meshTable.count();
    if (meshCount == 0)
        return true;  // Nothing to upload

    // Calculate total size needed
    VkDeviceSize totalVertexSize = 0;
    VkDeviceSize totalIndexSize  = 0;

    for (uint32_t i = 0; i < meshCount; i++)
    {
        const Mesh* mesh = data.meshTable.get(i);
        if (!mesh) continue;

        totalVertexSize += mesh->vertices.size() * sizeof(Vertex);
        totalIndexSize  += mesh->indices.size() * sizeof(uint32_t);
    }

    VkDeviceSize totalSize = totalVertexSize + totalIndexSize;
    if (totalSize == 0)
        return true;

    // Create single buffer for all mesh data
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = totalSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };

    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo allocInfo;
    if (vmaCreateBuffer(m_WindowData->allocator, &bufferCI, &allocCI,
                       &data.meshBuffer, &data.meshBufferAllocation, &allocInfo) != VK_SUCCESS)
    {
        std::cerr << "Failed to create mesh buffer\n";
        return false;
    }

    // Map and upload all mesh data
    char* bufferPtr = static_cast<char*>(allocInfo.pMappedData);
    VkDeviceSize vertexOffset = 0;
    VkDeviceSize indexOffset  = totalVertexSize;  // Indices start after all vertices

    data.meshGPUInfo.resize(meshCount);

    for (uint32_t i = 0; i < meshCount; i++)
    {
        const Mesh* mesh = data.meshTable.get(i);
        if (!mesh) continue;

        VkDeviceSize vertexSize = mesh->vertices.size() * sizeof(Vertex);
        VkDeviceSize indexSize  = mesh->indices.size() * sizeof(uint32_t);

        // Copy vertex data
        if (vertexSize > 0)
        {
            std::memcpy(bufferPtr + vertexOffset, mesh->vertices.data(), vertexSize);
        }

        // Copy index data
        if (indexSize > 0)
        {
            std::memcpy(bufferPtr + indexOffset, mesh->indices.data(), indexSize);
        }

        // Record GPU info for this mesh
        data.meshGPUInfo[i] = {
            .vertexOffset = vertexOffset,
            .indexOffset  = indexOffset,
            .vertexCount  = static_cast<uint32_t>(mesh->vertices.size()),
            .indexCount   = static_cast<uint32_t>(mesh->indices.size())
        };

        vertexOffset += vertexSize;
        indexOffset  += indexSize;
    }

    // VMA keeps the buffer mapped, so we don't need to unmap
    std::cout << "Uploaded " << meshCount << " meshes to GPU ("
              << totalVertexSize << " bytes vertices, "
              << totalIndexSize << " bytes indices)\n";

    return true;
}

void DefaultGameWorld::addMeshPrimitive(const std::string& name, Mesh mesh,
                                        const glm::vec3& position,
                                        const glm::vec4& color)
{
    // Add mesh to table
    uint32_t meshIndex = data.meshTable.add(name, std::move(mesh));

    // Re-upload all meshes to GPU (includes the new one)
    // Destroy old buffer first
    if (data.meshBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_WindowData->allocator, data.meshBuffer, data.meshBufferAllocation);

    if (!uploadMeshesToGPU())
    {
        std::cerr << "Failed to upload meshes to GPU after adding " << name << "\n";
        return;
    }

    // Create game object with the mesh
    GameObject obj{
        .name = name,
        .meshIndex = meshIndex
    };
    obj.components.add(std::make_unique<Transform>(
        position,
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f)
    ));
    obj.components.add(std::make_unique<MeshRenderer>(
        ColorMode::ObjectColor, color
    ));

    data.scene.addObject(std::move(obj));
}

void DefaultGameWorld::OnDetach()
{
    // Destroy mesh buffer
    if (data.meshBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_WindowData->allocator, data.meshBuffer, data.meshBufferAllocation);

    data.resources.destroy(m_WindowData->device, m_WindowData->allocator);
    data.pipeline.destroy(m_WindowData->device);
}

void DefaultGameWorld::OnUpdate(float ts)
{
    // Game logic would update m_Scene here (e.g., rotate objects, move camera)
    // For now, camera is static at default position
    (void)ts;
}

void DefaultGameWorld::OnRender(VkCommandBuffer cb, const glm::ivec2& windowSize, uint32_t frameIndex)
{
    // Update camera uniform buffer for this frame
    float aspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    data.resources.updateCameraBuffer(frameIndex, data.camera, aspectRatio);

    // Set viewport and scissor (dynamic state)
    VkViewport viewport{
        .x        = 0.0f,
        .y        = 0.0f,
        .width    = static_cast<float>(windowSize.x),
        .height   = static_cast<float>(windowSize.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(cb, 0, 1, &viewport);

    VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = { static_cast<uint32_t>(windowSize.x), static_cast<uint32_t>(windowSize.y) }
    };
    vkCmdSetScissor(cb, 0, 1, &scissor);

    // Bind pipeline
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline.pipeline);

    // Bind global descriptor set (camera UBO)
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, data.pipeline.layout,
                            0, 1, &data.resources.globalSets[frameIndex], 0, nullptr);

    // Draw each object in the scene
    if (data.meshBuffer == VK_NULL_HANDLE)
        return;  // No meshes uploaded yet

    for (const auto& obj : data.scene.objects)
    {
        // Skip objects without Transform component
        const Transform* transform = obj.components.get<Transform>();
        if (!transform)
            continue;

        // Skip objects without MeshRenderer component
        const MeshRenderer* renderer = obj.components.get<MeshRenderer>();
        if (!renderer)
            continue;

        // Validate mesh index
        if (obj.meshIndex >= data.meshGPUInfo.size())
            continue;

        const MeshGPUInfo& meshInfo = data.meshGPUInfo[obj.meshIndex];
        if (meshInfo.indexCount == 0)
            continue;

        // Prepare push constants with model matrix, color mode, and object color
        struct {
            glm::mat4 model;
            glm::vec4 objectColor;
            uint32_t colorMode;
            uint32_t padding[3];
        } pushData;

        pushData.model = transform->toMatrix();
        pushData.objectColor = renderer->objectColor;
        pushData.colorMode = (renderer->colorMode == ColorMode::ObjectColor) ? 1u : 0u;

        vkCmdPushConstants(cb, data.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                          0, sizeof(pushData), &pushData);

        // Bind vertex buffer
        VkDeviceSize vertexOffsets[] = { meshInfo.vertexOffset };
        vkCmdBindVertexBuffers(cb, 0, 1, &data.meshBuffer, vertexOffsets);

        // Bind index buffer
        vkCmdBindIndexBuffer(cb, data.meshBuffer, meshInfo.indexOffset, VK_INDEX_TYPE_UINT32);

        // Draw indexed
        vkCmdDrawIndexed(cb, meshInfo.indexCount, 1, 0, 0, 0);
    }
}
