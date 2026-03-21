#include "DecisionsLayer.h"
#include "imgui.h"
#include "../engine/sol_ImGui.h"
#include <iostream>
#include "../directory_tools.h"

DecisionsLayer::DecisionsLayer(const std::filesystem::path& path)
    : lua()
    , lua_view(lua.lua_state())
    , projectLocation(&path)
{
    luaL_openlibs(lua);
    scriptsDir = path / "Scripts" ;
}

DecisionsLayer::~DecisionsLayer() {}

void DecisionsLayer::OnAttach()
{
    sol_ImGui::Init(lua_view);

    lua.set_function("print", [](sol::variadic_args va) {
        for (auto v : va) {
            std::cout << v.as<std::string>() << "\t";
        }
        std::cout << "\n";
    });

    ScanScriptsFolder();
}

void DecisionsLayer::ScanScriptsFolder()
{
    if (!std::filesystem::exists(scriptsDir)) {
        std::filesystem::create_directories(scriptsDir);
        std::cout << "[Lua] Created scripts directory: " << scriptsDir << "\n";
        return;
    }

    std::filesystem::path mainPath = scriptsDir / mainFileName;

    if (directory_tools::dirExists(&mainPath)) {
        main.path = mainPath;
        main.lastModified = {};
        LoadScript(main);
    } else {
        std::cout << "[Lua] Error: Could not find main file: " << mainPath << "\n";
    }
}


void DecisionsLayer::LoadScript(LuaUIScript& script)
{
    script.valid = false;

    auto result = lua.safe_script_file(script.path.string(), sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error err = result;
        std::cerr << "[Lua] Load error in " << script.path.filename()
                  << ": " << err.what() << "\n";
        return;
    }

    // Require a global table called "main"
    sol::optional<sol::table> mainTable = lua["main"];
    if (!mainTable) {
        std::cerr << "[Lua] " << script.path.filename()
                  << " must define a global table called 'main'\n";
        return;
    }

    sol::table mod = mainTable.value();

    auto bindFn = [&](const char* name, sol::protected_function& target) -> bool {
        sol::optional<sol::protected_function> fn = mod[name];
        if (!fn) {
            std::cerr << "[Lua] " << script.path.filename()
                      << " missing required function: main." << name << "\n";
            return false;
        }
        target = fn.value();
        return true;
    };

    bool ok = true;
    ok &= bindFn("OnRender",   script.lau_onRender);
    ok &= bindFn("OnStart",    script.lau_onStart);
    ok &= bindFn("OnUpdate",   script.lau_onUpdate);
    ok &= bindFn("OnRenderUI", script.lau_onRenderUI);

    if (!ok) {
        std::cerr << "[Lua] " << script.path.filename()
                  << " failed to load — all four functions must be defined\n";
        return;
    }

    std::error_code ec;
    script.lastModified = std::filesystem::last_write_time(script.path, ec);
    script.valid = true;

    std::cout << "[Lua] Loaded: " << script.path.filename() << "\n";
}

void DecisionsLayer::CheckForChanges()
{
    if (!main.path.empty()) {
        std::error_code ec;
        auto currentTime = std::filesystem::last_write_time(main.path, ec);
        if (!ec && currentTime != main.lastModified) {
            std::cout << "[Lua] Reloading: " << main.path.filename() << "\n";
            LoadScript(main);
        }
    }
}

void DecisionsLayer::OnDetach() {}

void DecisionsLayer::OnStart()
{
    if (main.valid && main.lau_onStart.valid()) {
        auto result = main.lau_onStart();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[Lua] Runtime error in OnStart: " << err.what() << "\n";
        }
    }
}

void DecisionsLayer::OnUpdate(float ts)
{
    reloadTimer += ts;
    if (reloadTimer >= RELOAD_INTERVAL) {
        reloadTimer = 0.0f;
        CheckForChanges();
    }

    if (main.valid && main.lau_onUpdate.valid()) {
        auto result = main.lau_onUpdate(ts);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[Lua] Runtime error in OnUpdate: " << err.what() << "\n";
            main.valid = false;
        }
    }
}

void DecisionsLayer::OnUIRender()
{
    if (!main.valid) return;

    if (main.lau_onRenderUI.valid()) {
        auto result = main.lau_onRenderUI();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[Lua] Runtime error in OnRenderUI: " << err.what() << "\n";
            main.valid = false;
            return;
        }
    }

    if (main.lau_onRender.valid()) {
        auto result = main.lau_onRender();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "[Lua] Runtime error in OnRender: " << err.what() << "\n";
            main.valid = false;
        }
    }
}

void DecisionsLayer::OnRender(VkCommandBuffer cb, const glm::ivec2& windowSize, uint32_t frameIndex) {}

void DecisionsLayer::OnEvent(const SDL_Event& e) {}