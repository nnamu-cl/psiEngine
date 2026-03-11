#include <filesystem>
#include <iostream>
#include <vector>


#include "directory_tools.h"
#include "data_types/cashflow.h"
#include "engine/Game.h"

int main()
{
    // Load the data to be used (csv form)
    {
        char* value = std::getenv("HOME");
        std::filesystem::path path(value);
        path /= "Documents";
        path /= "psiProject";
        std::cout << path << std::endl;

        if (directory_tools::dirExists(&path))
        {
            std::cout << path << " exists." << std::endl;
        }
        else
        {
            std::cout << path << " does not exist. Now creating the folder. " << std::endl;
            std::filesystem::create_directories(path);

            if (directory_tools::dirExists(&path))
            {
                std::cout << path << " exists." << std::endl;
            }
        }


        // We want the following kind of data
        // Expected Incomes
        // Expected Expenses

        /*std::filesystem::path cashflows_path = path / "cashflows.csv";

        Game game(0, 0);
        if (game.LoadCashFlows(&cashflows_path, TimeFormat::ISO8601))
        {
            game.PrintHead();
        }
        else
        {
            std::cout << "Failed to load cashflows from " << cashflows_path << std::endl;
        }

        std::filesystem::path saveFilePath = path / "saveFile.gameFile";
        std::cout << "Saving the created game to the file:  " << saveFilePath << std::endl;

        Game::SaveGame(game, &saveFilePath);*/


        std::filesystem::path saveFilePath = path / "saveFile.txt";
        Game game(0, 0);

        Game::LoadGame(game, &saveFilePath);


        game.PrintHead();




    }
    // Pack data into correct datasets - probably in financial terms
    // Perform needed calculations
    // Print out/visualize important information

    return 0;
}
