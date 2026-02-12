#include "DefaultGameWorld.h"
#include "ApplicationWindow.h"   // full definition of ApplicationWindowData
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"
#include "Components/LineRenderer.h"
#include "Data/LineRendererData.h"
#include <iostream>
#include <cstring>

#include "imgui.h"

// Forward declaration of LineRendererNode to set callback
#include "../../nodeGraph/nodes/ObjectNodes.h"

DefaultGameWorld::DefaultGameWorld(ApplicationWindowData* windowData)
    : m_WindowData{ windowData }
{}

void DefaultGameWorld::OnAttach()
{
    // Set up LineRendererNode callback to create line data
    LineRendererNode::s_CreateLineCallback = [this]() -> LineRendererData* {
        // Create a new GameObject with LineRenderer component
        auto lineData = std::make_unique<LineRendererData>();
        LineRendererData* dataPtr = lineData.get();

        // Store the data (ownership)
        data.lineDataStorage.push_back(std::move(lineData));

        // Create game object with line renderer
        GameObject obj{
            .name = "Line_" + std::to_string(data.lineDataStorage.size()),
            .meshIndex = 0xFFFFFFFF  // No mesh
        };

        obj.components.add(std::make_unique<Transform>(
            glm::vec3(0.0f),
            glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f)
        ));

        auto lineRenderer = std::make_unique<LineRenderer>(dataPtr);
        obj.components.add(std::move(lineRenderer));

        data.scene.addObject(std::move(obj));

        std::cout << "LineRendererNode callback: Created line object with data at " << dataPtr << "\n";

        return dataPtr;
    };

    // Create resources (descriptor layouts, pool, uniform buffers)
    if (!data.resources.create(m_WindowData->device,
                                m_WindowData->allocator,
                                maxFramesInFlight))
    {
        std::cerr << "Failed to create DefaultGameWorld resources\n";
        return;
    }

    // Create all pipeline variants
    std::vector<VkDescriptorSetLayout> setLayouts = { data.resources.globalSetLayout };
    if (!data.pipelineManager.create(m_WindowData->device,
                                     setLayouts,
                                     m_WindowData->swapchainImageFormat,
                                     m_WindowData->depthFormat))
    {
        std::cerr << "Failed to create DefaultGameWorld pipelines\n";
        return;
    }

    // Create line pipeline
    LinePipelineDesc linePipelineDesc{
        .vertexShaderPath = "libs/graphics/window/assets/shaders/DefaultGameWorld/line.vert.slang",
        .geometryShaderPath = "libs/graphics/window/assets/shaders/DefaultGameWorld/line.geom.slang",
        .fragmentShaderPath = "libs/graphics/window/assets/shaders/DefaultGameWorld/line.frag.slang",
        .topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
        .depthTest = true
    };

    if (!data.linePipeline.create(m_WindowData->device,
                                   linePipelineDesc,
                                   setLayouts,
                                   m_WindowData->swapchainImageFormat,
                                   m_WindowData->depthFormat))
    {
        std::cerr << "Failed to create line pipeline\n";
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

void DefaultGameWorld::addLinePrimitive(const std::string& name,
                                        const std::vector<glm::vec3>& points,
                                        const glm::vec4& color)
{
    std::cout << "addLinePrimitive called: " << name << " with " << points.size() << " points\n";

    // Re-upload all lines to GPU (will include the new one)
    if (data.lineBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_WindowData->allocator, data.lineBuffer, data.lineBufferAllocation);

    // Create line data
    auto lineData = std::make_unique<LineRendererData>();
    lineData->points = points;
    lineData->properties.color = color;
    LineRendererData* dataPtr = lineData.get();

    // Store the data (ownership)
    data.lineDataStorage.push_back(std::move(lineData));

    // Create game object with line renderer
    GameObject obj{
        .name = name,
        .meshIndex = 0xFFFFFFFF  // No mesh
    };

    obj.components.add(std::make_unique<Transform>(
        glm::vec3(0.0f),
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f)
    ));

    auto lineRenderer = std::make_unique<LineRenderer>(dataPtr);
    obj.components.add(std::move(lineRenderer));

    data.scene.addObject(std::move(obj));

    std::cout << "Scene now has " << data.scene.objects.size() << " objects\n";

    // Upload lines to GPU
    if (!uploadLinesToGPU())
    {
        std::cerr << "Failed to upload lines to GPU after adding " << name << "\n";
    }
    else
    {
        std::cout << "Successfully uploaded lines. lineGPUInfo size: " << data.lineGPUInfo.size() << "\n";
        std::cout << "lineBuffer: " << (data.lineBuffer != VK_NULL_HANDLE ? "valid" : "NULL") << "\n";
    }
}

bool DefaultGameWorld::uploadLinesToGPU()
{
    std::cout << "uploadLinesToGPU called\n";

    // Collect all line vertex data from LineRenderer components
    std::vector<std::vector<LineVertex>> allLineData;
    data.lineGPUInfo.clear();

    for (const auto& obj : data.scene.objects)
    {
        const LineRenderer* lineRenderer = obj.components.get<LineRenderer>();
        if (!lineRenderer || !lineRenderer->data)
        {
            continue;
        }

        std::cout << "Found LineRenderer on object: " << obj.name << "\n";
        allLineData.push_back(lineRenderer->buildVertexData());
    }

    std::cout << "Found " << allLineData.size() << " lines to upload\n";

    if (allLineData.empty())
        return true;  // No lines to upload

    // Calculate total size needed
    VkDeviceSize totalSize = 0;
    for (const auto& lineData : allLineData)
    {
        totalSize += lineData.size() * sizeof(LineVertex);
    }

    if (totalSize == 0)
        return true;

    // Create single buffer for all line data
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = totalSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo allocInfo;
    if (vmaCreateBuffer(m_WindowData->allocator, &bufferCI, &allocCI,
                       &data.lineBuffer, &data.lineBufferAllocation, &allocInfo) != VK_SUCCESS)
    {
        std::cerr << "Failed to create line buffer\n";
        return false;
    }

    // Map and upload all line data
    char* bufferPtr = static_cast<char*>(allocInfo.pMappedData);
    VkDeviceSize offset = 0;

    for (const auto& lineData : allLineData)
    {
        VkDeviceSize dataSize = lineData.size() * sizeof(LineVertex);

        if (dataSize > 0)
        {
            std::memcpy(bufferPtr + offset, lineData.data(), dataSize);
        }

        // Record GPU info for this line
        data.lineGPUInfo.push_back({
            .vertexOffset = offset,
            .vertexCount  = static_cast<uint32_t>(lineData.size())
        });

        offset += dataSize;
    }

    std::cout << "Uploaded " << allLineData.size() << " lines to GPU (" << totalSize << " bytes)\n";

    return true;
}

void DefaultGameWorld::OnDetach()
{
    // Destroy mesh buffer
    if (data.meshBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_WindowData->allocator, data.meshBuffer, data.meshBufferAllocation);

    // Destroy line buffer
    if (data.lineBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_WindowData->allocator, data.lineBuffer, data.lineBufferAllocation);

    data.resources.destroy(m_WindowData->device, m_WindowData->allocator);
    data.pipelineManager.destroy(m_WindowData->device);
    data.linePipeline.destroy(m_WindowData->device);
}

void DefaultGameWorld::OnUpdate(float ts)
{
    // Game logic would update m_Scene here (e.g., rotate objects, move camera)
    // For now, camera is static at default position
    (void)ts;

    // Check if any line renderers need GPU update
    bool needsLineUpdate = false;
    for (const auto& obj : data.scene.objects)
    {
        const LineRenderer* lineRenderer = obj.components.get<LineRenderer>();
        if (lineRenderer && lineRenderer->data && lineRenderer->data->needsGPUUpdate)
        {
            needsLineUpdate = true;
            lineRenderer->data->needsGPUUpdate = false;  // Works because needsGPUUpdate is mutable
        }
    }

    if (needsLineUpdate)
    {
        uploadLinesToGPU();
    }
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

    // Draw each object in the scene
    if (data.meshBuffer == VK_NULL_HANDLE && data.lineBuffer == VK_NULL_HANDLE)
        return;  // No meshes or lines uploaded yet

    // Track current pipeline to minimize state changes
    Pipeline* currentPipeline = nullptr;

    // Render meshes
    if (data.meshBuffer != VK_NULL_HANDLE && !data.meshGPUInfo.empty())
    {
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

        // Get appropriate pipeline for this material
        Pipeline* pipeline = data.pipelineManager.getPipeline(renderer->material);
        if (!pipeline)
            continue;

        // Bind pipeline if it changed
        if (pipeline != currentPipeline)
        {
            vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout,
                                   0, 1, &data.resources.globalSets[frameIndex], 0, nullptr);
            currentPipeline = pipeline;
        }

        // Prepare extended push constants
        struct {
            glm::mat4 model;             // 64 bytes (offset 0)
            glm::vec4 objectColor;       // 16 bytes (offset 64)
            float emissionIntensity;     // 4 bytes (offset 80)
            float _pad1[3];              // 12 bytes padding for vec3 alignment (offset 84)
            glm::vec3 tintColor;         // 12 bytes (offset 96, aligned to 16)
            float alphaCutoff;           // 4 bytes (offset 108)
            uint32_t colorMode;          // 4 bytes (offset 112)
            uint32_t shadingMode;        // 4 bytes (offset 116)
            uint32_t padding[2];         // 8 bytes (offset 120)
        } pushData;

        pushData.model = transform->toMatrix();
        pushData.objectColor = renderer->material.objectColor;
        pushData.emissionIntensity = renderer->material.emissionIntensity;
        pushData._pad1[0] = 0.0f;
        pushData._pad1[1] = 0.0f;
        pushData._pad1[2] = 0.0f;
        pushData.tintColor = renderer->material.tintColor;
        pushData.alphaCutoff = renderer->material.alphaCutoff;
        pushData.colorMode = (renderer->material.colorMode == ColorMode::ObjectColor) ? 1u : 0u;
        pushData.shadingMode = (renderer->material.shadingMode == ShadingMode::Unlit) ? 1u : 0u;
        pushData.padding[0] = 0;
        pushData.padding[1] = 0;

        vkCmdPushConstants(cb, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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

    // Render lines
    if (data.lineBuffer != VK_NULL_HANDLE && !data.lineGPUInfo.empty())
    {
        static bool firstFrame = true;
        if (firstFrame) {
            std::cout << "Rendering lines: " << data.lineGPUInfo.size() << " line objects\n";
            firstFrame = false;
        }

        // Bind line pipeline
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, data.linePipeline.pipeline);
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, data.linePipeline.layout,
                               0, 1, &data.resources.globalSets[frameIndex], 0, nullptr);

        size_t lineIndex = 0;
        for (const auto& obj : data.scene.objects)
        {
            const LineRenderer* lineRenderer = obj.components.get<LineRenderer>();
            if (!lineRenderer || !lineRenderer->data)
                continue;

            const Transform* transform = obj.components.get<Transform>();
            if (!transform)
                continue;

            if (lineIndex >= data.lineGPUInfo.size())
                break;

            const LineGPUInfo& lineInfo = data.lineGPUInfo[lineIndex++];
            if (lineInfo.vertexCount < 2)
                continue;

            // Prepare line push constants
            struct {
                glm::mat4 viewProj;          // 64 bytes
                glm::vec4 globalColor;       // 16 bytes
                float globalThickness;       // 4 bytes
                float dashLength;            // 4 bytes
                float gapLength;             // 4 bytes
                uint32_t lineStyle;          // 4 bytes
                uint32_t antiAlias;          // 4 bytes
                float smoothness;            // 4 bytes
                uint32_t padding[6];         // 24 bytes
            } linePushData;

            linePushData.viewProj = data.camera.projectionMatrix(aspectRatio) * data.camera.viewMatrix();
            linePushData.globalColor = lineRenderer->data->properties.color;
            linePushData.globalThickness = 1.0f;  // Thickness is per-vertex
            linePushData.dashLength = lineRenderer->data->properties.dashLength;
            linePushData.gapLength = lineRenderer->data->properties.gapLength;
            linePushData.lineStyle = static_cast<uint32_t>(lineRenderer->data->properties.style);
            linePushData.antiAlias = lineRenderer->data->properties.antiAlias ? 1u : 0u;
            linePushData.smoothness = lineRenderer->data->properties.smoothness;
            std::memset(linePushData.padding, 0, sizeof(linePushData.padding));

            vkCmdPushConstants(cb, data.linePipeline.layout,
                             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
                             0, sizeof(linePushData), &linePushData);

            // Bind vertex buffer
            VkDeviceSize offsets[] = { lineInfo.vertexOffset };
            vkCmdBindVertexBuffers(cb, 0, 1, &data.lineBuffer, offsets);

            // Draw lines (topology is always line strip for now)
            vkCmdDraw(cb, lineInfo.vertexCount, 1, 0, 0);
        }
    }
}
