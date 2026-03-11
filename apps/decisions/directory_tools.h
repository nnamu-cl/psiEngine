#pragma once
#include <filesystem>



namespace directory_tools
{
    inline bool dirExists(const std::filesystem::path* path )
    {
        return std::filesystem::exists(*path);
    }

}

