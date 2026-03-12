#define SOL_ALL_SAFE_OPTS 1
#define SOL_USING_CXX_LUA 0
#define SOL_USING_CXX_LUAJIT 0
#include <filesystem>
#include <iostream>
#include <vector>

#include "directory_tools.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "luajit.h"
}

#include "engine/Game.h"
#include "sol2/sol.hpp"


static int wrap_exceptions(lua_State *L, lua_CFunction f)
{
    try {
        return f(L);  // Call wrapped function and return result.
    } catch (const char *s) {  // Catch and convert exceptions.
        lua_pushstring(L, s);
    } catch (std::exception& e) {
        lua_pushstring(L, e.what());
    } catch (...) {
        lua_pushliteral(L, "caught (...)");
    }
    return lua_error(L);  // Rethrow as a Lua error.
}

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


        sol::state lua;
        if (!lua.lua_state()) {
            std::cerr << "Failed to create lua state!" << std::endl;
            return 1;
        }
        lua.open_libraries(sol::lib::base, sol::lib::package);
        lua.script("print('bark bark bark!')");



    }
    // Pack data into correct datasets - probably in financial terms
    // Perform needed calculations
    // Print out/visualize important information

    return 0;
}
