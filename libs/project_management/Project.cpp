#include "include/Project.h"

#include <filesystem>


namespace fs = std::filesystem;

Project::Project(std::string dir)
    : directory(dir)
{
    current = this;  //Make sure to save us as the currently active project
}


fs::path Project::GetUserHome() {

    const char* home = std::getenv("USERPROFILE");

    if (home) {
        return fs::path(home);
    }

//TODO: here implement cross platform way to handle getting the home path

    return fs::path();
}
