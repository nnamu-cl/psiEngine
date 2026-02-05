//
// Created by namus on 2/4/2026.
//

#include "ApplicationWindow.h"
#include "Application.h"
#include "Layer.h"
#include "imgui.h"

class DemoLayer : public Application::Layer
{
public:
    void OnUIRender() override
    {
        ImGui::ShowDemoWindow();
    }
};

int main()
{
    ApplicationWindowSpecifications windowSpecs{1920, 1080, "Main Window",
                                                SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE};

    ApplicationWindow window(windowSpecs);
    window.Init();

    Application::Application app;
    DemoLayer demo;
    app.PushLayer(&demo);

    window.Start(app);

    return 0;
}
