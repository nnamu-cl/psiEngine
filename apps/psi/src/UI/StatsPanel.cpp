#include "StatsPanel.h"

#include "ApplicationWindow.h"
#include "layers/PsiWorldLayer.h"
#include "imgui.h"

StatsPanel::StatsPanel(PsiWorldLayer* worldLayer)
    : m_WorldLayer(worldLayer)
{
}

void StatsPanel::Render()
{
    if (!m_Visible)
        return;

    // Position at top right as an overlay
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 windowPos = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 10.0f, viewport->WorkPos.y + 50.0f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));

    // Overlay style flags
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0);

    ImGui::Begin("Statistics", &m_Visible, windowFlags);


    // Frame stats
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Spacing();
    ImGui::Text("Objects: %zu", m_WorldLayer->data.scene.objects.size());
    ImGui::Text("Time: %.3f", ApplicationWindow::instance->currentTime);
    ImGui::End();
}
