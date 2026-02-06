#pragma once

#include "SkinsBase.h"
#include <unordered_map>
#include <string>

namespace Application::Skins {
    class ShadSkin : public ImGuiSkin {
    public:
        void ApplySkin() override;

        // Get a specific font by name
        static ImFont* GetFont(const std::string& name);

    private:
        static std::unordered_map<std::string, ImFont*> s_Fonts;
    };
}
