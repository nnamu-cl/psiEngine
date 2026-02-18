#pragma once
#include <filesystem>
#include <string>


namespace fs = std::filesystem;

class Project {
public:
    const std::string directory;

    explicit Project(std::string dir);

    static fs::path GetUserHome();

};
