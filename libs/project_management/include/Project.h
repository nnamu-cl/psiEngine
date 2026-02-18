#pragma once
#include <filesystem>
#include <string>


namespace fs = std::filesystem;

class Project {
public:
    inline static Project *current = nullptr; //the current project right now
    const std::string directory;

    explicit Project(std::string dir);

    static fs::path GetUserHome();

    static Project *Get() { return current; }
};
