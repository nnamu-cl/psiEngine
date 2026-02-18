#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

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

private:
    static PsiProjectList s_List;
};
