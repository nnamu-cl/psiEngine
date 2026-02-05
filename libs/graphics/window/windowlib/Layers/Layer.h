#pragma once

#include <glm/vec2.hpp>
#include <volk/volk.h>

namespace Application {

class Layer
{
public:
    virtual ~Layer() = default;

    virtual void OnAttach() {}
    virtual void OnDetach() {}

    virtual void OnUpdate(float ts) {}
    virtual void OnRender(VkCommandBuffer cb, const glm::ivec2& windowSize, uint32_t frameIndex) {}
    virtual void OnUIRender() {}
};

} // namespace Walnut
