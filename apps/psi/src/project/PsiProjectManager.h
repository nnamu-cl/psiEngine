#pragma once

#include <Project.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Forward declaration — avoids pulling the full NodeSystem header into every
// translation unit that includes this file.
class NodeGraph;
class NodeSystemDrawer;

// Metadata for a single psi project on disk
struct PsiProjectEntry {
    std::string name;
    std::string directory;
    std::string lastOpened;  // ISO 8601, e.g. "2026-02-18T14:00:00"
};

// Top-level document stored in psi_projects.json
struct PsiProjectList {
    std::string engineVersion = "0.1.0";
    std::vector<PsiProjectEntry> projects;
};

// Manages the psi-specific project registry.
// Owns the location and format of the project list file —
// project_management provides the utilities (GetUserHome, etc.),
// but psi decides what to do with them.
class PsiProjectManager {
public:
    // Root folder where psi engine metadata lives:
    //   ~/Documents/psiEngine/
    static fs::path GetMetadataDir();

    // Full path to the project list file:
    //   ~/Documents/psiEngine/psi_projects.json
    static fs::path GetProjectsFilePath();

    // -----------------------------------------------------------------------
    // Project registry — load/save the list of known projects (psi_projects.json)
    // These are NOT the same as SaveProject/LoadProject below.
    // -----------------------------------------------------------------------

    // Load project list from disk. Safe to call even if the file doesn't exist yet.
    static void Load();

    // Persist the current project list to disk.
    static void Save();

    // Register a project. If one with the same directory already exists,
    // its lastOpened timestamp is refreshed instead.
    static void AddProject(const std::string& name, const std::string& directory);

    // Remove a project by its directory path.
    static void RemoveProject(const std::string& directory);

    static const std::vector<PsiProjectEntry>& GetProjects();

    // Returns nullptr if not found.
    static PsiProjectEntry* FindProject(const std::string& directory);

    // -----------------------------------------------------------------------
    // Active project — the project currently open in the editor
    // -----------------------------------------------------------------------

    // Set the node graph that SaveProject/LoadProject will operate on.
    // Call this once in main() after the node editor layer is constructed.
    static void SetNodeGraph(NodeGraph* graph);

    // Set the node system drawer for saving/loading editor layout.
    // Call this once in main() after the node editor layer is attached.
    static void SetNodeSystemDrawer(NodeSystemDrawer* drawer);

    // Mark a project as the currently-open project.
    // Creates a Project object from the given name + directory and stores it.
    static void SetCurrentProject(const std::string& name, const std::string& dir);

    // Returns the currently-open project, or nullptr if none is open.
    static Project* GetCurrentProject();

    // -----------------------------------------------------------------------
    // Project content I/O — save/load the actual node graph on disk.
    // These are distinct from Save()/Load() which only touch the registry.
    //
    //   SaveProject  ->  writes  {currentProject->directory}/graph.json
    //   LoadProject  ->  reads   {currentProject->directory}/graph.json
    //
    // Both are no-ops if no current project or no node graph is set.
    // -----------------------------------------------------------------------
    static void SaveProject();
    static void LoadProject();

private:
    static PsiProjectList          s_List;
    static std::unique_ptr<Project> s_CurrentProject;
    static NodeGraph*              s_NodeGraph;
    static NodeSystemDrawer*       s_NodeSystemDrawer;
};
