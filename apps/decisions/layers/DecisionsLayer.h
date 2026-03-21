#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <vulkan/vulkan_core.h>

#include "imgui_impl_sdl3.h"
#include "../sol2/sol.hpp"
#include "glm/vec2.hpp"
#include "Layers/Layer.h"

struct LuaUIScript {
    std::filesystem::path path;
    std::filesystem::file_time_type lastModified;
    sol::protected_function lau_onRender;
    sol::protected_function lau_onStart;
    sol::protected_function lau_onUpdate;
    sol::protected_function lau_onRenderUI;
    bool valid = false;
};

class DecisionsLayer : public Application::Layer {
public:
    DecisionsLayer(const std::filesystem::path& projectPath);
    ~DecisionsLayer();

    void OnAttach();
    void OnDetach();
    void OnUpdate(float ts);
    void OnUIRender();
    void OnRender(VkCommandBuffer cb, const glm::ivec2& windowSize, uint32_t frameIndex);
    void OnEvent(const SDL_Event& e);
    void OnStart();

private:
    void ScanScriptsFolder();
    void LoadScript(LuaUIScript& script);
    void CheckForChanges();

    sol::state lua;
    sol::state_view lua_view;
    const std::filesystem::path* projectLocation;

    std::string mainFileName = "main.lua";
    std::filesystem::path scriptsDir;
    LuaUIScript main;

    float reloadTimer = 0.0f;
    static constexpr float RELOAD_INTERVAL = 0.5f;
};