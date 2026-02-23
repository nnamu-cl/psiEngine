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

DefaultGameWorld::DefaultGameWorld(ApplicationWindowData *windowData)
    : windowData{windowData} {

    data.windowData = windowData; //save the window data as we might need it later


}

void DefaultGameWorld::OnAttach() {
    // Set up LineRendererNode callback to create line data
    LineRendererNode::s_CreateLineCallback = [this]() -> LineRendererData * {
        // Create a new GameObject with LineRenderer component
        auto lineData = std::make_unique<LineRendererData>();
        LineRendererData *dataPtr = lineData.get();

        // Store the data (ownership)
        data.lineDataStorage.push_back(std::move(lineData));

        // Create game object with line renderer
        GameObject obj{
            .name = "Line_" + std::to_string(data.lineDataStorage.size()),
            .meshIndex = 0xFFFFFFFF // No mesh
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
    if (!data.resources.create(windowData->device,
                               windowData->allocator,
                               maxFramesInFlight)) {
        std::cerr << "Failed to create DefaultGameWorld resources\n";
        return;
    }

    // Create all pipeline variants
    std::vector<VkDescriptorSetLayout> setLayouts = {data.resources.globalSetLayout};
    if (!data.pipelineManager.create(windowData->device,
                                     setLayouts,
                                     windowData->swapchainImageFormat,
                                     windowData->depthFormat)) {
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

    //Loading and compilation happens here
    if (!data.linePipeline.create(windowData->device,
                                  linePipelineDesc,
                                  setLayouts,
                                  windowData->swapchainImageFormat,
                                  windowData->depthFormat)) {
        std::cerr << "Failed to create line pipeline\n";
        return;
    }

    // Initialise camera controller against the window's camera and SDL window
    data.cameraController.init(&data.camera, windowData->sdlWindow);

    // Scene starts empty - meshes can be added via UI
    std::cout << "DefaultGameWorld layer attached successfully\n";
    std::cout << "  Scene ready - use UI to add objects\n";
}

bool DefaultGameWorld::uploadMeshesToGPU() {
    uint32_t meshCount = data.meshTable.count();
    if (meshCount == 0)
        return true; // Nothing to upload

    // Calculate total size needed
    VkDeviceSize totalVertexSize = 0;
    VkDeviceSize totalIndexSize = 0;

    for (uint32_t i = 0; i < meshCount; i++) {
        const Mesh *mesh = data.meshTable.get(i);
        if (!mesh) continue;

        totalVertexSize += mesh->vertices.size() * sizeof(Vertex);
        totalIndexSize += mesh->indices.size() * sizeof(uint32_t);
    }

    VkDeviceSize totalSize = totalVertexSize + totalIndexSize;
    if (totalSize == 0)
        return true;

    // Create single buffer for all mesh data
    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = totalSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };

    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                 VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo allocInfo;
    if (vmaCreateBuffer(windowData->allocator, &bufferCI, &allocCI,
                        &data.meshBuffer, &data.meshBufferAllocation, &allocInfo) != VK_SUCCESS) {
        std::cerr << "Failed to create mesh buffer\n";
        return false;
    }

    // Map and upload all mesh data
    char *bufferPtr = static_cast<char *>(allocInfo.pMappedData);
    VkDeviceSize vertexOffset = 0;
    VkDeviceSize indexOffset = totalVertexSize; // Indices start after all vertices

    data.meshGPUInfo.resize(meshCount);

    for (uint32_t i = 0; i < meshCount; i++) {
        const Mesh *mesh = data.meshTable.get(i);
        if (!mesh) continue;

        VkDeviceSize vertexSize = mesh->vertices.size() * sizeof(Vertex);
        VkDeviceSize indexSize = mesh->indices.size() * sizeof(uint32_t);

        // Copy vertex data
        if (vertexSize > 0) {
            std::memcpy(bufferPtr + vertexOffset, mesh->vertices.data(), vertexSize);
        }

        // Copy index data
        if (indexSize > 0) {
            std::memcpy(bufferPtr + indexOffset, mesh->indices.data(), indexSize);
        }

        // Record GPU info for this mesh
        data.meshGPUInfo[i] = {
            .vertexOffset = vertexOffset,
            .indexOffset = indexOffset,
            .vertexCount = static_cast<uint32_t>(mesh->vertices.size()),
            .indexCount = static_cast<uint32_t>(mesh->indices.size())
        };

        vertexOffset += vertexSize;
        indexOffset += indexSize;
    }

    // VMA keeps the buffer mapped, so we don't need to unmap
    std::cout << "Uploaded " << meshCount << " meshes to GPU ("
            << totalVertexSize << " bytes vertices, "
            << totalIndexSize << " bytes indices)\n";

    return true;
}

void DefaultGameWorld::addMeshPrimitive(const std::string &name, Mesh mesh,
                                        const glm::vec3 &position,
                                        const glm::vec4 &color) {
    // Add mesh to table
    uint32_t meshIndex = data.meshTable.add(name, std::move(mesh));

    // Re-upload all meshes to GPU (includes the new one)
    // Destroy old buffer first
    if (data.meshBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(windowData->allocator, data.meshBuffer, data.meshBufferAllocation);

    if (!uploadMeshesToGPU()) {
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

void DefaultGameWorld::addLinePrimitive(const std::string &name,
                                        const std::vector<glm::vec3> &points,
                                        const glm::vec4 &color) {
    std::cout << "addLinePrimitive called: " << name << " with " << points.size() << " points\n";

    // Re-upload all lines to GPU (will include the new one)
    if (data.lineBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(windowData->allocator, data.lineBuffer, data.lineBufferAllocation);

    // Create line data
    auto lineData = std::make_unique<LineRendererData>();
    lineData->points = points;
    lineData->properties.color = color;
    LineRendererData *dataPtr = lineData.get();

    // Store the data (ownership)
    data.lineDataStorage.push_back(std::move(lineData));

    // Create game object with line renderer
    GameObject obj{
        .name = name,
        .meshIndex = 0xFFFFFFFF // No mesh
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
    if (!data.linePipeline.UploadLinesToGPU(data, windowData->allocator)) {
        std::cerr << "Failed to upload lines to GPU after adding " << name << "\n";
    } else {
        std::cout << "Successfully uploaded lines. lineGPUInfo size: " << data.lineGPUInfo.size() << "\n";
        std::cout << "lineBuffer: " << (data.lineBuffer != VK_NULL_HANDLE ? "valid" : "NULL") << "\n";
    }
}


void DefaultGameWorld::OnDetach() {
    // Destroy mesh buffer
    if (data.meshBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(windowData->allocator, data.meshBuffer, data.meshBufferAllocation);

    // Destroy line buffer
    if (data.lineBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(windowData->allocator, data.lineBuffer, data.lineBufferAllocation);

    data.resources.destroy(windowData->device, windowData->allocator);
    data.pipelineManager.destroy(windowData->device);
    data.linePipeline.destroy(windowData->device);
}

void DefaultGameWorld::OnEvent(const SDL_Event& e) {
    data.cameraController.onEvent(e);
}

void DefaultGameWorld::OnUpdate(float ts) {
    data.cameraController.update(ts);

    // Check if any line renderers need GPU update
    bool needsLineUpdate = false;
    for (const auto &obj: data.scene.objects) {
        const LineRenderer *lineRenderer = obj.components.get<LineRenderer>();
        if (lineRenderer && lineRenderer->data && lineRenderer->data->needsGPUUpdate) {
            needsLineUpdate = true;
            lineRenderer->data->needsGPUUpdate = false; // Works because needsGPUUpdate is mutable
        }
    }

    if (needsLineUpdate) {
        data.linePipeline.UploadLinesToGPU(data, windowData->allocator);
    }
}

void DefaultGameWorld::OnRender(VkCommandBuffer cb, const glm::ivec2 &windowSize, uint32_t frameIndex) {
    // Update camera uniform buffer for this frame
    float aspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    data.resources.updateCameraBuffer(frameIndex, data.camera, aspectRatio);

    // Set viewport and scissor (dynamic state)
    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(windowSize.x),
        .height = static_cast<float>(windowSize.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(cb, 0, 1, &viewport);

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = {static_cast<uint32_t>(windowSize.x), static_cast<uint32_t>(windowSize.y)}
    };
    vkCmdSetScissor(cb, 0, 1, &scissor);

    // Draw each object in the scene
    if (data.meshBuffer == VK_NULL_HANDLE && data.lineBuffer == VK_NULL_HANDLE)
        return; // No meshes or lines uploaded yet

    // Track current pipeline to minimize state changes
    Pipeline *currentPipeline = nullptr;

    // Render meshes
    if (data.meshBuffer != VK_NULL_HANDLE && !data.meshGPUInfo.empty()) {
        for (const auto &obj: data.scene.objects) {
            // Skip objects without Transform component
            const Transform *transform = obj.components.get<Transform>();
            if (!transform)
                continue;

            // Skip objects without MeshRenderer component
            const MeshRenderer *renderer = obj.components.get<MeshRenderer>();
            if (!renderer)
                continue;

            // Validate mesh index
            if (obj.meshIndex >= data.meshGPUInfo.size())
                continue;

            const MeshGPUInfo &meshInfo = data.meshGPUInfo[obj.meshIndex];
            if (meshInfo.indexCount == 0)
                continue;

            // Get appropriate pipeline for this material
            Pipeline *pipeline = data.pipelineManager.getPipeline(renderer->material);
            if (!pipeline)
                continue;

            // Bind pipeline if it changed
            if (pipeline != currentPipeline) {
                vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
                vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout,
                                        0, 1, &data.resources.globalSets[frameIndex], 0, nullptr);
                currentPipeline = pipeline;
            }

            // Prepare extended push constants
            struct {
                glm::mat4 model; // 64 bytes (offset 0)
                glm::vec4 objectColor; // 16 bytes (offset 64)
                float emissionIntensity; // 4 bytes (offset 80)
                float _pad1[3]; // 12 bytes padding for vec3 alignment (offset 84)
                glm::vec3 tintColor; // 12 bytes (offset 96, aligned to 16)
                float alphaCutoff; // 4 bytes (offset 108)
                uint32_t colorMode; // 4 bytes (offset 112)
                uint32_t shadingMode; // 4 bytes (offset 116)
                uint32_t padding[2]; // 8 bytes (offset 120)
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

            //TODO: Change this to send this using the
            vkCmdPushConstants(cb, pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0, sizeof(pushData), &pushData);

            // Bind vertex buffer
            VkDeviceSize vertexOffsets[] = {meshInfo.vertexOffset};
            vkCmdBindVertexBuffers(cb, 0, 1, &data.meshBuffer, vertexOffsets);

            // Bind index buffer
            vkCmdBindIndexBuffer(cb, data.meshBuffer, meshInfo.indexOffset, VK_INDEX_TYPE_UINT32);

            // Draw indexed
            vkCmdDrawIndexed(cb, meshInfo.indexCount, 1, 0, 0, 0);
        }
    }



    //Tell the line renderer pipeline to render
    data.linePipeline.DoRender(cb, frameIndex, aspectRatio, data);


}
