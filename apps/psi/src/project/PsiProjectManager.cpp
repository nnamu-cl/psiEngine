#include "PsiProjectManager.h"

#include <Project.h>         // Project::GetUserHome()
#include <glaze/glaze.hpp>
#include "nodes/NodeSystem.h"
#include "drawers/NodeSystemDrawer.h"

#include <chrono>
#include <fstream>
#include <sstream>

// -----------------------------------------------------------------------
// Glaze will auto-reflect these structs because they are plain aggregates
// (all public, no user-defined constructors).
// -----------------------------------------------------------------------

PsiProjectList           PsiProjectManager::s_List;
std::unique_ptr<Project> PsiProjectManager::s_CurrentProject  = nullptr;
NodeGraph*               PsiProjectManager::s_NodeGraph        = nullptr;
NodeSystemDrawer*        PsiProjectManager::s_NodeSystemDrawer = nullptr;

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

static std::string CurrentISO8601() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    std::ostringstream ss;
    ss << (tm.tm_year + 1900) << '-'
       << (tm.tm_mon  + 1 < 10 ? "0" : "") << (tm.tm_mon + 1) << '-'
       << (tm.tm_mday     < 10 ? "0" : "") << tm.tm_mday << 'T'
       << (tm.tm_hour     < 10 ? "0" : "") << tm.tm_hour << ':'
       << (tm.tm_min      < 10 ? "0" : "") << tm.tm_min  << ':'
       << (tm.tm_sec      < 10 ? "0" : "") << tm.tm_sec;
    return ss.str();
}

// -----------------------------------------------------------------------
// PsiProjectManager
// -----------------------------------------------------------------------

fs::path PsiProjectManager::GetMetadataDir() {
    return Project::GetUserHome() / "Documents" / "psiEngine";
}

fs::path PsiProjectManager::GetProjectsFilePath() {
    return GetMetadataDir() / "psi_projects.json";
}

void PsiProjectManager::Load() {
    fs::path path = GetProjectsFilePath();
    if (!fs::exists(path)) {
        return;  // Nothing saved yet — start with an empty list
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return;
    }

    std::string buffer((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    PsiProjectList loaded;
    auto err = glz::read_json(loaded, buffer);
    if (!err) {
        s_List = std::move(loaded);
    }
}

void PsiProjectManager::Save() {
    fs::path dir = GetMetadataDir();
    fs::create_directories(dir);

    std::string buffer;
    auto err = glz::write<glz::opts{.prettify = true}>(s_List, buffer);
    if (err) {
        return;
    }

    std::ofstream file(GetProjectsFilePath(), std::ios::trunc);
    file << buffer;
}

void PsiProjectManager::AddProject(const std::string& name, const std::string& directory) {
    // Refresh timestamp if the project already exists
    if (auto* existing = FindProject(directory)) {
        existing->lastOpened = CurrentISO8601();
        existing->name       = name;
        return;
    }

    s_List.projects.push_back(PsiProjectEntry{
        .name        = name,
        .directory   = directory,
        .lastOpened  = CurrentISO8601(),
    });
}

void PsiProjectManager::RemoveProject(const std::string& directory) {
    auto& v = s_List.projects;
    v.erase(std::remove_if(v.begin(), v.end(),
        [&](const PsiProjectEntry& e) { return e.directory == directory; }),
        v.end());
}

const std::vector<PsiProjectEntry>& PsiProjectManager::GetProjects() {
    return s_List.projects;
}

PsiProjectEntry* PsiProjectManager::FindProject(const std::string& directory) {
    for (auto& e : s_List.projects) {
        if (e.directory == directory) return &e;
    }
    return nullptr;
}

// -----------------------------------------------------------------------
// Active project
// -----------------------------------------------------------------------

void PsiProjectManager::SetNodeGraph(NodeGraph* graph) {
    s_NodeGraph = graph;
}

void PsiProjectManager::SetNodeSystemDrawer(NodeSystemDrawer* drawer) {
    s_NodeSystemDrawer = drawer;
}

void PsiProjectManager::SetCurrentProject(const std::string& name, const std::string& dir) {
    s_CurrentProject = std::make_unique<Project>(dir);
}

Project* PsiProjectManager::GetCurrentProject() {
    return s_CurrentProject.get();
}

// -----------------------------------------------------------------------
// Project content I/O
// Distinct from Save()/Load() which only touch the project registry.
// -----------------------------------------------------------------------

void PsiProjectManager::SaveProject() {
    if (!s_NodeGraph || !s_CurrentProject) return;
    s_NodeGraph->Save(s_CurrentProject->directory);

    if (s_NodeSystemDrawer) {
        fs::path rendererFile = fs::path(s_CurrentProject->directory) / "node_renderer.json";
        s_NodeSystemDrawer->Save(rendererFile.string());
    }
}

void PsiProjectManager::LoadProject() {
    if (!s_NodeGraph || !s_CurrentProject) return;
    fs::path graphFile = fs::path(s_CurrentProject->directory) / "graph.json";
    s_NodeGraph->Load(graphFile.string());

    if (s_NodeSystemDrawer) {
        fs::path rendererFile = fs::path(s_CurrentProject->directory) / "node_renderer.json";
        s_NodeSystemDrawer->Load(rendererFile.string());
    }
}
