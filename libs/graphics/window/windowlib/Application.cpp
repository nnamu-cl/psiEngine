#include "Application.h"

#include <algorithm>

namespace Application {

void Application::PushLayer(Layer* layer)
{
    m_LayerStack.push_back(layer);
    layer->OnAttach();
}

void Application::PopLayer(Layer* layer)
{
    auto it = std::find(m_LayerStack.begin(), m_LayerStack.end(), layer);
    if (it != m_LayerStack.end())
    {
        layer->OnDetach();
        m_LayerStack.erase(it);
    }
}

}
