#pragma once
#include <filesystem>



namespace directory_tools
{
    inline bool dirExists(const std::filesystem::path* path )
    {
        return std::filesystem::exists(*path);
    }


    //Ensures that the main engine folder exists
    inline bool ensureMainFolder(const std::filesystem::path& path) {

        if (dirExists(&path))
        {
            std::cout << path << " exists." << std::endl;
            return true;
        }
        else
        {
            std::cout << path << " does not exist. Now creating the folder. " << std::endl;
            std::filesystem::create_directories(path);

            if (dirExists(&path))
            {
                std::cout << path << " exists." << std::endl;
            }

            return true;
        }

        return false;

    }

}

