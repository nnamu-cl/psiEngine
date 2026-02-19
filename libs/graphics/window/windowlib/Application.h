#pragma once

#include <vector>
#include <glm/fwd.hpp>

#include "Layers/Layer.h"

namespace Application {

class Application
{
public:
    void PushLayer(Layer* layer);
    void PopLayer(Layer* layer);

    const std::vector<Layer*>& GetLayerStack() const { return m_LayerStack; }
    VkClearColorValue clearColorValue  {0.176f, 0.180f, 0.196f, 1.0f};

private:
    std::vector<Layer*> m_LayerStack;
};

} // namespace Application
