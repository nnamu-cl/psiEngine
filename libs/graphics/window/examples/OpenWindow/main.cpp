//
// Created by namus on 2/4/2026.
//

#include "ApplicationWindow.h"
#include "Application.h"
#include "../../windowlib/Layers/Layer.h"
#include "imgui.h"
#include "ImGuiSkins/ShadSkin.h"
#include "ImGuiSkins/SkinsBase.h"
#include "Layers/DefaultGameWorld/DefaultGameWorld.h"

class DemoNodeGraph : public Application::Layer
{
public:
    void OnUIRender() override
    {
        ImGui::ShowDemoWindow();
    }
};

int main()
{
    ApplicationWindowSpecifications windowSpecs{1920, 1080, "Im3DDemo",
                                                SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE};

    ApplicationWindow window(windowSpecs);
    window.Init();

    Application::Application app;


    //Apply out theme
    Application::Skins::ShadSkin shadSkin;
    shadSkin.ApplySkin();

    // Create layers
    DemoNodeGraph guiLayer; // GUI layer
    DefaultGameWorld gameWorld (&window.data); // World Layer



    // Attach layers
    app.PushLayer(&gameWorld);
    app.PushLayer(&guiLayer);


    window.Start(app);
    return 0;
}
