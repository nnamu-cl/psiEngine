#include "PsiUILayer.h"
#include "layers/PsiWorldLayer.h"
#include "Application.h"
#include "UI/ControlPanel.h"
#include "UI/StatsPanel.h"
#include "UI/InspectorPanel.h"
#include "imgui.h"
#include <string>

// Generator library for procedural mesh generation
#include <generator/BoxMesh.hpp>
#include <generator/SphereMesh.hpp>
#include <generator/CylinderMesh.hpp>
#include <generator/PlaneMesh.hpp>
#include <generator/ConeMesh.hpp>
#include <generator/TorusMesh.hpp>

PsiUILayer::PsiUILayer(PsiWorldLayer* worldLayer, Application::Application* app)
    : m_WorldLayer(worldLayer)
    , m_Application(app)
{
    // Create UI panels
    m_ControlPanel = std::make_unique<ControlPanel>(worldLayer, app);
    m_StatsPanel = std::make_unique<StatsPanel>(worldLayer);
    m_InspectorPanel = std::make_unique<InspectorPanel>(worldLayer);
}

PsiUILayer::~PsiUILayer()
{
    // Panels will be automatically destroyed via unique_ptr
}

void PsiUILayer::OnAttach()
{
    // Called when the layer is attached to the application
    // Initialize UI resources, load settings, etc.
}

void PsiUILayer::OnDetach()
{
    // Called when the layer is detached from the application
    // Cleanup UI resources, save settings, etc.
}

void PsiUILayer::OnUpdate(float ts)
{
    // Update logic called every frame
    // ts = time step (delta time) in seconds
    // Update animations, time-based UI effects, etc.
}

void PsiUILayer::OnUIRender()
{
    // Render the UI using ImGui
    RenderMenuBar();

    // Render all panels
    m_ControlPanel->Render();
    m_StatsPanel->Render();
    m_InspectorPanel->Render();

    if (m_ShowDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_ShowDemoWindow);
    }
}

void PsiUILayer::RenderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::MenuItem("Cube"))
            {
                AddPrimitiveMesh("Cube");
            }
            if (ImGui::MenuItem("Sphere"))
            {
                AddPrimitiveMesh("Sphere");
            }
            if (ImGui::MenuItem("Cylinder"))
            {
                AddPrimitiveMesh("Cylinder");
            }
            if (ImGui::MenuItem("Plane"))
            {
                AddPrimitiveMesh("Plane");
            }
            if (ImGui::MenuItem("Cone"))
            {
                AddPrimitiveMesh("Cone");
            }
            if (ImGui::MenuItem("Torus"))
            {
                AddPrimitiveMesh("Torus");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void PsiUILayer::AddPrimitiveMesh(const std::string& primitiveType)
{
    Mesh newMesh;
    std::string meshName = primitiveType;

    // Helper lambda to convert generator mesh to our Mesh format
    auto convertGeneratorMesh = [&newMesh](auto generatorMesh) {
        // Generator uses a lazy evaluation system with generators
        // We iterate through vertices and triangles to extract the data

        // Extract vertices
        auto vertices = generatorMesh.vertices();
        for (const auto& v : vertices) {
            Vertex vertex;
            vertex.position = glm::vec3(v.position[0], v.position[1], v.position[2]);
            vertex.normal = glm::vec3(v.normal[0], v.normal[1], v.normal[2]);
            vertex.texCoord = glm::vec2(v.texCoord[0], v.texCoord[1]);
            newMesh.vertices.push_back(vertex);
        }

        // Extract triangles (indices)
        auto triangles = generatorMesh.triangles();
        for (const auto& tri : triangles) {
            newMesh.indices.push_back(tri.vertices[0]);
            newMesh.indices.push_back(tri.vertices[1]);
            newMesh.indices.push_back(tri.vertices[2]);
        }
    };

    // Generate the appropriate primitive mesh
    if (primitiveType == "Cube") {
        generator::BoxMesh boxMesh;
        convertGeneratorMesh(boxMesh);
    }
    else if (primitiveType == "Sphere") {
        generator::SphereMesh sphereMesh;
        convertGeneratorMesh(sphereMesh);
    }
    else if (primitiveType == "Cylinder") {
        generator::CylinderMesh cylinderMesh;
        convertGeneratorMesh(cylinderMesh);
    }
    else if (primitiveType == "Plane") {
        generator::PlaneMesh planeMesh;
        convertGeneratorMesh(planeMesh);
    }
    else if (primitiveType == "Cone") {
        generator::ConeMesh coneMesh;
        convertGeneratorMesh(coneMesh);
    }
    else if (primitiveType == "Torus") {
        generator::TorusMesh torusMesh;
        convertGeneratorMesh(torusMesh);
    }

    // Generate a unique name for this instance
    static int primitiveCounter = 0;
    std::string uniqueName = meshName + "_" + std::to_string(primitiveCounter++);

    // Add mesh to the mesh table
    uint32_t meshIndex = m_WorldLayer->data.meshTable.add(uniqueName, newMesh);

    // Upload meshes to GPU (required after adding new meshes)
    m_WorldLayer->uploadMeshesToGPU();

    // Create a GameObject with this mesh
    GameObject newObject;
    newObject.name = uniqueName;
    newObject.meshIndex = meshIndex;
    newObject.materialIndex = 0; // Use default material
    newObject.transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    newObject.transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

    // Add to scene
    m_WorldLayer->data.scene.addObject(newObject);
}
