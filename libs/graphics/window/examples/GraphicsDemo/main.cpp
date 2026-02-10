#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>

#include "Application.h"
#include "ApplicationWindow.h"
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"
#include "Components/LineRenderer.h"
#include "Layers/DefaultGameWorld/DefaultGameWorld.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>



class BasicGraphicsLayer : public DefaultGameWorld{
public:
    GameObject* mainGameObject = nullptr;


    explicit BasicGraphicsLayer(ApplicationWindowData *windowData)
        : DefaultGameWorld(windowData) {
    }




    void OnUIRender() override {

        if (mainGameObject != nullptr) {

             Transform* transform = mainGameObject->components.get<Transform>();
             LineRenderer* lineRenderer = mainGameObject->components.get<LineRenderer>();
             // MeshRenderer* renderer = mainGameObject->components.get<MeshRenderer>();

             // Draw ImGui control window
             ImGui::Begin("Object Controls");

             if (transform != nullptr) {
                 ImGui::Text("Transform");
                 ImGui::Separator();
                 ImGui::DragFloat3("Position", glm::value_ptr(transform->position), 0.1f);

                 glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(transform->rotation));
                 if (ImGui::DragFloat3("Rotation", glm::value_ptr(eulerAngles), 1.0f)) {
                     transform->rotation = glm::quat(glm::radians(eulerAngles));
                 }

                 ImGui::DragFloat3("Scale", glm::value_ptr(transform->scale), 0.01f, 0.001f, 100.0f);
                 ImGui::Spacing();
             }

             if (lineRenderer != nullptr) {
                 ImGui::Text("Line Renderer");
                 ImGui::Separator();
                 lineRenderer->OnInspectorGUI();
             }

             /*
             // Commented out mesh renderer UI
             if (renderer != nullptr) {
                 ImGui::Text("Mesh Renderer");
                 ImGui::Separator();

                 // Shading Mode
                 const char* shadingModes[] = { "Lit", "Unlit" };
                 int currentShadingMode = (renderer->material.shadingMode == ShadingMode::Lit) ? 0 : 1;
                 if (ImGui::Combo("Shading Mode", &currentShadingMode, shadingModes, 2)) {
                     renderer->material.shadingMode = (currentShadingMode == 0) ? ShadingMode::Lit : ShadingMode::Unlit;
                 }

                 ImGui::Spacing();

                 // Color Mode
                 const char* colorModes[] = { "Vertex Color", "Object Color" };
                 int currentMode = (renderer->material.colorMode == ColorMode::VertexColor) ? 0 : 1;
                 if (ImGui::Combo("Color Mode", &currentMode, colorModes, 2)) {
                     renderer->material.colorMode = (currentMode == 0) ? ColorMode::VertexColor : ColorMode::ObjectColor;
                 }

                 if (renderer->material.colorMode == ColorMode::ObjectColor) {
                     ImGui::ColorEdit4("Object Color", glm::value_ptr(renderer->material.objectColor));
                 }

                 ImGui::Spacing();
                 ImGui::Separator();
                 ImGui::Text("Material Properties");
                 ImGui::Separator();

                 // Tier 1: Core
                 ImGui::SliderFloat("Emission Intensity", &renderer->material.emissionIntensity, 0.0f, 5.0f);

                 // Tier 2: Highly recommended
                 ImGui::ColorEdit3("Tint Color", glm::value_ptr(renderer->material.tintColor));

                 const char* blendModes[] = { "Opaque", "Transparent", "Additive", "Multiply" };
                 int currentBlendMode = static_cast<int>(renderer->material.blendMode);
                 if (ImGui::Combo("Blend Mode", &currentBlendMode, blendModes, 4)) {
                     renderer->material.blendMode = static_cast<BlendMode>(currentBlendMode);
                 }

                 // Tier 3: Nice-to-have
                 ImGui::SliderFloat("Alpha Cutoff", &renderer->material.alphaCutoff, 0.0f, 1.0f);
                 ImGui::Checkbox("Double-Sided", &renderer->material.doubleSided);
             }
             */

             ImGui::End();
        }
    }
};


int main() {

    // Create a new Application Window

    ApplicationWindowSpecifications specifications{
    1920, 1080, "Graphics Demo - Line Renderer"
    };
    ApplicationWindow window(specifications);
    window.Init();
    Application::Application application;


    BasicGraphicsLayer basicGraphicsLayer(&window.data);

    // Commented out cube creation
    /*
    auto mesh = MeshTable::unitCube();
    basicGraphicsLayer.addMeshPrimitive("Cube", std::move(mesh),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    */

    // Create a simple spiral line
    std::cout << "Creating spiral line...\n";
    std::vector<glm::vec3> linePoints;
    const int numPoints = 50;
    const float radius = 2.0f;
    const float height = 4.0f;

    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        float angle = t * 4.0f * glm::pi<float>();  // 2 full rotations
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        float y = -height * 0.5f + height * t;
        linePoints.push_back(glm::vec3(x, y, z));
    }

    std::cout << "Created " << linePoints.size() << " line points. Calling addLinePrimitive...\n";
    basicGraphicsLayer.addLinePrimitive("Spiral Line", linePoints,
        glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));  // Cyan color
    std::cout << "addLinePrimitive returned\n";


    basicGraphicsLayer.mainGameObject = &basicGraphicsLayer.data.scene.objects[0];



    application.PushLayer(&basicGraphicsLayer);


    // Feed meshes into the window (without needing to import any node graph crap etc)
    window.Start(application);


    return 0;
}
