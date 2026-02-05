#pragma once

#include <vector>
#include "Layers/Layer.h"

namespace Application {

class Application
{
public:
    void PushLayer(Layer* layer);
    void PopLayer(Layer* layer);

    const std::vector<Layer*>& GetLayerStack() const { return m_LayerStack; }

private:
    std::vector<Layer*> m_LayerStack;
};

} // namespace Application
