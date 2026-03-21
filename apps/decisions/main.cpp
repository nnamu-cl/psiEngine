#define SOL_ALL_SAFE_OPTS 1
#define SOL_USING_CXX_LUA 0
#define SOL_USING_CXX_LUAJIT 0
#include <filesystem>
#include <iostream>
#include <vector>

#include "Application.h"
#include "ApplicationWindow.h"
#include "directory_tools.h"
#include "engine/colors/DecisionColors.h"
#include "ImGuiSkins/ShadSkin.h"
#include "layers/DecisionsLayer.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "luajit.h"
}

#include "engine/Game.h"
#include "sol2/sol.hpp"


static int wrap_exceptions(lua_State* L, lua_CFunction f)
{
    try
    {
        return f(L); // Call wrapped function and return result.
    }
    catch (const char* s)
    {
        // Catch and convert exceptions.
        lua_pushstring(L, s);
    }
    catch (std::exception& e)
    {
        lua_pushstring(L, e.what());
    }
    catch (...)
    {
        lua_pushliteral(L, "caught (...)");
    }
    return lua_error(L); // Rethrow as a Lua error.
}

int main()
{
    // Window specifications for PSI application
    ApplicationWindowSpecifications windowSpecs{
        1920,
        1080,
        "decisions",
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    };


    // Initialize the application window
    ApplicationWindow window(windowSpecs);
    if (!window.Init())
    {
        std::cerr << "Failed to initialize application window!" << std::endl;
        return -1;
    }


    // Create the application instance
    Application::Application app;

    // Set the background clear color to PSI secondary background
    app.clearColorValue.float32[0] = decision::Colors::Background3D.r;
    app.clearColorValue.float32[1] = decision::Colors::Background3D.g;
    app.clearColorValue.float32[2] = decision::Colors::Background3D.b;
    app.clearColorValue.float32[3] = 1.0f;

    // Apply UI theme (ShadSkin for consistent look)
    Application::Skins::ShadSkin shadSkin;
    shadSkin.ApplySkin();


    char* value = std::getenv("HOME");
    std::filesystem::path path(value);
    path /= "Documents";
    path /= "psiProject";
    std::cout << path << std::endl;

    if (!directory_tools::ensureMainFolder(path)) {
        std::cout << "Failed to find/create main folder!" << std::endl;
        return -1;
    }

    DecisionsLayer mainLayer (path);



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


    std::filesystem::path saveFilePath = path / "saveFile.save";
    Game game(0, 0);

    Game::LoadGame(game, &saveFilePath);


    game.PrintHead();


    app.PushLayer(&mainLayer);


    // Pack data into correct datasets - probably in financial terms
    // Perform needed calculations
    // Print out/visualize important information


    window.Start(app);

    return 0;
}
