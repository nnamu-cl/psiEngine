#include "ProjectHub.h"
#include "PsiProjectManager.h"
#include "imgui.h"
#include <nfd.h>
#include <filesystem>
#include <cstdio>

ProjectHub::ProjectHub() = default;

bool ProjectHub::Render()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700.0f, 500.0f), ImGuiCond_Always);

    constexpr ImGuiWindowFlags hubFlags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove;

    if (!ImGui::Begin("psiQuantum  -  Project Hub", nullptr, hubFlags))
    {
        ImGui::End();
        return false;
    }

    bool shouldClose          = false;
    bool openNewProjectDialog = false;

    // ----------------------------------------------------------------
    // Header: title + "New Project" button aligned to the right
    // ----------------------------------------------------------------
    ImGui::Text("Recent Projects");
    {
        constexpr float btnW = 130.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - btnW - ImGui::GetStyle().WindowPadding.x);
        if (ImGui::Button("New Project", ImVec2(btnW, 0)))
            openNewProjectDialog = true;
    }
    ImGui::Separator();

    // ----------------------------------------------------------------
    // Scrollable project list (leaves 50 px for the footer bar)
    // ----------------------------------------------------------------
    if (ImGui::BeginChild("ProjectList", ImVec2(0.0f, -50.0f)))
        RenderProjectList(shouldClose);
    ImGui::EndChild();

    // Open popups AFTER EndChild so they share the correct ID-stack level
    // with their matching BeginPopupModal calls below.
    if (openNewProjectDialog)
    {
        memset(m_NewProjectName,     0, sizeof(m_NewProjectName));
        auto defaultDir = (PsiProjectManager::GetMetadataDir() / "projects").string();
        snprintf(m_NewProjectLocation, sizeof(m_NewProjectLocation),
                 "%s", defaultDir.c_str());
        m_NewProjectError.clear();
        ImGui::OpenPopup("New Project##dialog");
    }
    if (m_PendingDeleteConfirm)
    {
        ImGui::OpenPopup("Delete Project##confirm");
        m_PendingDeleteConfirm = false;
    }

    // ----------------------------------------------------------------
    // Footer
    // ----------------------------------------------------------------
    ImGui::Separator();

    // ----------------------------------------------------------------
    // Modals
    // ----------------------------------------------------------------
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    RenderNewProjectModal(shouldClose);

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    RenderDeleteConfirmModal();

    ImGui::End();
    return shouldClose;
}

// ----------------------------------------------------------------
// Project list
// ----------------------------------------------------------------
void ProjectHub::RenderProjectList(bool& shouldClose)
{
    const auto& projects = PsiProjectManager::GetProjects();

    if (projects.empty())
    {
        ImGui::Spacing();
        ImGui::TextDisabled("No recent projects. Create a new one to get started.");
        return;
    }

    std::string openName, openDir;
    std::string pendingDeleteName, pendingDeleteDir;

    for (const auto& p : projects)
    {
        ImGui::PushID(p.directory.c_str());

        const bool  dirExists  = std::filesystem::exists(p.directory);
        const float winW       = ImGui::GetWindowWidth();
        const float padX       = ImGui::GetStyle().WindowPadding.x;
        constexpr float openBtnW   = 80.0f;
        constexpr float deleteBtnW = 75.0f;
        constexpr float btnSpacing =  8.0f;

        // Project name (with missing-directory indicator)
        if (!dirExists)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::Text("%s  [directory missing]", p.name.c_str());
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::Text("%s", p.name.c_str());
        }

        // Right-aligned Open + Delete buttons
        const float deleteStart = winW - padX - deleteBtnW;
        const float openStart   = deleteStart - btnSpacing - openBtnW;

        ImGui::SameLine(openStart);
        if (!dirExists) ImGui::BeginDisabled();
        if (ImGui::Button("Open", ImVec2(openBtnW, 0)))
        {
            openName = p.name;
            openDir  = p.directory;
        }
        if (!dirExists) ImGui::EndDisabled();

        ImGui::SameLine(deleteStart);
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.85f, 0.10f, 0.10f, 1.0f));
        if (ImGui::Button("Delete", ImVec2(deleteBtnW, 0)))
        {
            pendingDeleteName = p.name;
            pendingDeleteDir  = p.directory;
        }
        ImGui::PopStyleColor(3);

        ImGui::TextDisabled("%s", p.directory.c_str());
        ImGui::TextDisabled("Last opened: %s", p.lastOpened.c_str());
        ImGui::Separator();

        ImGui::PopID();
    }

    if (!openDir.empty())
    {
        PsiProjectManager::AddProject(openName, openDir);
        PsiProjectManager::Save();
        PsiProjectManager::SetCurrentProject(openName, openDir);
        PsiProjectManager::LoadProject();
        shouldClose = true;
    }

    // Defer popup opening until after EndChild (called from Render())
    if (!pendingDeleteDir.empty())
    {
        m_DeleteConfirmName    = pendingDeleteName;
        m_DeleteConfirmDir     = pendingDeleteDir;
        m_PendingDeleteConfirm = true;
    }
}

// ----------------------------------------------------------------
// New-project modal
// ----------------------------------------------------------------
void ProjectHub::RenderNewProjectModal(bool& shouldClose)
{
    namespace fs = std::filesystem;

    if (!ImGui::BeginPopupModal("New Project##dialog", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        return;

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
        const nfdu8char_t* defaultPath =
            m_NewProjectLocation[0] != '\0' ? m_NewProjectLocation : nullptr;
        nfdu8char_t* outPath = nullptr;
        if (NFD_PickFolderU8(&outPath, defaultPath) == NFD_OKAY)
        {
            snprintf(m_NewProjectLocation, sizeof(m_NewProjectLocation),
                     "%s", outPath);
            NFD_FreePathU8(outPath);
            m_NewProjectError.clear();
        }
    }

    // Live path preview
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
                PsiProjectManager::SetCurrentProject(name, projectDir.string());
                ImGui::CloseCurrentPopup();
                shouldClose = true;
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

// ----------------------------------------------------------------
// Delete-confirmation modal
// ----------------------------------------------------------------
void ProjectHub::RenderDeleteConfirmModal()
{
    if (!ImGui::BeginPopupModal("Delete Project##confirm", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        return;

    ImGui::Text("Remove project \"%s\"?", m_DeleteConfirmName.c_str());
    ImGui::Spacing();
    ImGui::TextDisabled("%s", m_DeleteConfirmDir.c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Option 1: remove from the registry only
    if (ImGui::Button("Remove from list", ImVec2(150.0f, 0.0f)))
    {
        PsiProjectManager::RemoveProject(m_DeleteConfirmDir);
        PsiProjectManager::Save();
        m_DeleteConfirmName.clear();
        m_DeleteConfirmDir.clear();
        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    // Option 2: remove from registry AND delete files from disk
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.85f, 0.10f, 0.10f, 1.0f));
    if (ImGui::Button("Delete files too", ImVec2(150.0f, 0.0f)))
    {
        std::error_code ec;
        std::filesystem::remove_all(m_DeleteConfirmDir, ec);
        PsiProjectManager::RemoveProject(m_DeleteConfirmDir);
        PsiProjectManager::Save();
        m_DeleteConfirmName.clear();
        m_DeleteConfirmDir.clear();
        ImGui::CloseCurrentPopup();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(80.0f, 0.0f)))
    {
        m_DeleteConfirmName.clear();
        m_DeleteConfirmDir.clear();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
