#include "PsiUILayer.h"
#include "layers/PsiWorldLayer.h"
#include "Application.h"
#include "project/PsiProjectManager.h"
#include <filesystem>
#include <cstdio>
#include <nfd.h>
#include "UI/ControlPanel.h"
#include "UI/StatsPanel.h"
#include "UI/InspectorPanel.h"
#include "Layers/DefaultGameWorld/Mesh.h"
#include "imgui.h"
#include <string>
#include <generator/BoxMesh.hpp>
#include <generator/TriangleMesh.hpp>
#include <generator/SphereMesh.hpp>
#include <generator/CylinderMesh.hpp>
#include <generator/TorusMesh.hpp>
#include <generator/ConeMesh.hpp>

namespace {
    // Helper function to convert generator mesh to our Mesh format
    template<typename GeneratorMesh>
    Mesh convertFromGenerator(GeneratorMesh&& genMesh)
    {
        Mesh mesh;

        // Extract vertices
        auto vertexGen = genMesh.vertices();
        while (!vertexGen.done()) {
            auto v = vertexGen.generate();
            mesh.vertices.push_back({
                glm::vec3(static_cast<float>(v.position[0]),
                         static_cast<float>(v.position[1]),
                         static_cast<float>(v.position[2])),
                glm::vec3(static_cast<float>(v.normal[0]),
                         static_cast<float>(v.normal[1]),
                         static_cast<float>(v.normal[2])),
                glm::vec2(static_cast<float>(v.texCoord[0]),
                         static_cast<float>(v.texCoord[1])),
                glm::vec4(1.0f)
            });
            vertexGen.next();
        }

        // Extract indices
        auto triangleGen = genMesh.triangles();
        while (!triangleGen.done()) {
            auto tri = triangleGen.generate();
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[0]));
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[1]));
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[2]));
            triangleGen.next();
        }

        return mesh;
    }
}

PsiUILayer::PsiUILayer(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer, Application::Application* app)
    : m_WorldLayer(worldLayer)
    , m_NodeEditorLayer(nodeEditorLayer)
    , m_Application(app)
{
    // Create UI panels
    m_ControlPanel = std::make_unique<ControlPanel>(worldLayer, nodeEditorLayer, app);
    m_StatsPanel = std::make_unique<StatsPanel>(worldLayer);
    m_InspectorPanel = std::make_unique<InspectorPanel>(worldLayer, nodeEditorLayer);
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
    // Project hub is shown on startup; hides itself once the user picks or skips
    if (m_ShowProjectHub)
    {
        RenderProjectHub();
        return; // don't render the rest of the UI while the hub is visible
    }

    // Render panels (StatsPanel last so it appears on top)
    m_ControlPanel->Render();
    m_InspectorPanel->Render();

    // Render StatsPanel last so it's on top
    m_StatsPanel->Render();
}

void PsiUILayer::RenderProjectHub()
{
    namespace fs = std::filesystem;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Centre a fixed-size window in the viewport
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700.0f, 500.0f), ImGuiCond_Always);

    constexpr ImGuiWindowFlags hubFlags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove;

    if (!ImGui::Begin("psiQuantum  -  Project Hub", nullptr, hubFlags))
    {
        ImGui::End();
        return;
    }

    // ----------------------------------------------------------------
    // Header: title text + "New Project" button aligned to the right
    // ----------------------------------------------------------------
    ImGui::Text("Recent Projects");
    {
        constexpr float btnW = 130.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - btnW - ImGui::GetStyle().WindowPadding.x);

        if (ImGui::Button("New Project", ImVec2(btnW, 0)))
        {
            // Reset dialog state and fill in a sensible default location
            memset(m_NewProjectName,     0, sizeof(m_NewProjectName));
            auto defaultDir = (PsiProjectManager::GetMetadataDir() / "projects").string();
            snprintf(m_NewProjectLocation, sizeof(m_NewProjectLocation),
                     "%s", defaultDir.c_str());
            m_NewProjectError.clear();
            ImGui::OpenPopup("New Project##dialog");
        }
    }
    ImGui::Separator();

    // ----------------------------------------------------------------
    // Scrollable project list (leaves 50 px for the footer bar)
    // ----------------------------------------------------------------
    if (ImGui::BeginChild("ProjectList", ImVec2(0.0f, -50.0f)))
    {
        const auto& projects = PsiProjectManager::GetProjects();

        if (projects.empty())
        {
            ImGui::Spacing();
            ImGui::TextDisabled("No recent projects. Create a new one to get started.");
        }
        else
        {
            // Collect the open request outside the loop to avoid iterator issues
            std::string openName, openDir;

            for (const auto& p : projects)
            {
                ImGui::PushID(p.directory.c_str());

                constexpr float btnW = 80.0f;
                const float     winW = ImGui::GetWindowWidth();
                const float     padX = ImGui::GetStyle().WindowPadding.x;

                // Project name + Open button on the same row
                ImGui::Text("%s", p.name.c_str());
                ImGui::SameLine(winW - btnW - padX);
                if (ImGui::Button("Open", ImVec2(btnW, 0)))
                {
                    openName = p.name;
                    openDir  = p.directory;
                }

                ImGui::TextDisabled("%s", p.directory.c_str());
                ImGui::TextDisabled("Last opened: %s", p.lastOpened.c_str());
                ImGui::Separator();

                ImGui::PopID();
            }

            // Apply the open request after the loop
            if (!openDir.empty())
            {
                PsiProjectManager::AddProject(openName, openDir); // refreshes timestamp
                PsiProjectManager::Save();
                m_ShowProjectHub = false;
            }
        }
    }
    ImGui::EndChild();

    // ----------------------------------------------------------------
    // Footer
    // ----------------------------------------------------------------
    ImGui::Separator();


    // ================================================================
    // New Project modal (opened by the button above)
    // ================================================================
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("New Project##dialog", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Project Name:");
        ImGui::SetNextItemWidth(420.0f);
        ImGui::InputText("##pname", m_NewProjectName, sizeof(m_NewProjectName));

        ImGui::Spacing();
        ImGui::Text("Parent folder (a subfolder named after your project will be created here):");
        ImGui::SetNextItemWidth(330.0f);
        ImGui::InputText("##ploc", m_NewProjectLocation, sizeof(m_NewProjectLocation));
        ImGui::SameLine();
        if (ImGui::Button("Browse..."))
        {
            // Pass the current text as the starting directory so the dialog opens there
            const nfdu8char_t* defaultPath =
                m_NewProjectLocation[0] != '\0' ? m_NewProjectLocation : nullptr;

            nfdu8char_t* outPath = nullptr;
            nfdresult_t result = NFD_PickFolderU8(&outPath, defaultPath);
            if (result == NFD_OKAY)
            {
                snprintf(m_NewProjectLocation, sizeof(m_NewProjectLocation),
                         "%s", outPath);
                NFD_FreePathU8(outPath);
                m_NewProjectError.clear();
            }
        }

        // Live preview of the resulting path
        {
            std::string preview;
            if (m_NewProjectName[0] != '\0' && m_NewProjectLocation[0] != '\0')
                preview = (fs::path(m_NewProjectLocation) / m_NewProjectName).string();
            else if (m_NewProjectLocation[0] != '\0')
                preview = (fs::path(m_NewProjectLocation) / "[project name]").string();
            else
                preview = "(enter details above)";
            ImGui::TextDisabled("Will create: %s", preview.c_str());
        }

        // Validation error message
        if (!m_NewProjectError.empty())
        {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
            ImGui::TextWrapped("%s", m_NewProjectError.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Create", ImVec2(120.0f, 0.0f)))
        {
            std::string name(m_NewProjectName);
            std::string loc(m_NewProjectLocation);

            if (name.empty())
            {
                m_NewProjectError = "Project name cannot be empty.";
            }
            else if (loc.empty())
            {
                m_NewProjectError = "Parent folder cannot be empty.";
            }
            else
            {
                fs::path projectDir = fs::path(loc) / name;
                std::error_code ec;
                fs::create_directories(projectDir, ec);

                if (ec)
                {
                    m_NewProjectError = "Failed to create folder: " + ec.message();
                }
                else
                {
                    PsiProjectManager::AddProject(name, projectDir.string());
                    PsiProjectManager::Save();
                    m_ShowProjectHub = false;
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f)))
        {
            m_NewProjectError.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

