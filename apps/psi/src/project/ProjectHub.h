#pragma once

#include <string>

class ProjectHub
{
public:
    ProjectHub();

    // Renders the hub. Returns true when the user has opened/created a project
    // and the hub should be dismissed.
    bool Render();

private:
    // New-project dialog state
    char        m_NewProjectName[256]     = {};
    char        m_NewProjectLocation[512] = {};
    std::string m_NewProjectError;

    // Delete-confirmation state
    std::string m_DeleteConfirmName;
    std::string m_DeleteConfirmDir;
    bool        m_PendingDeleteConfirm = false;

    void RenderProjectList(bool& shouldClose);
    void RenderNewProjectModal(bool& shouldClose);
    void RenderDeleteConfirmModal();
};
