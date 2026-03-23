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

    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Statistics", &m_Visible);


    // Frame stats
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Spacing();
    ImGui::Text("Objects: %zu", m_WorldLayer->data.scene.objects.size());
    ImGui::Text("Time: %.3f", ApplicationWindow::instance->currentTime);
    ImGui::End();
}
