#include "Atom.h"
#include "imgui.h"

void Atom::OnInspectorGUI()
{
    if (!data)
    {
        ImGui::Text("No atom data attached");
        return;
    }

    ImGui::Indent();
    DrawAtomUI(data);
    ImGui::Unindent();
}

void Atom::DrawAtomUI(AtomData* data, bool compact)
{
    if (!data)
    {
        ImGui::Text("No atom data attached");
        return;
    }

    bool changed = false;
    float itemWidth = compact ? 150.0f : 150.0f;
    float labelX = compact ? 0.0f : 120.0f;

    auto label = [&](const char* text) {
        ImGui::Text("%s", text);
        if (!compact) { ImGui::SameLine(); ImGui::SetCursorPosX(labelX); }
        ImGui::SetNextItemWidth(itemWidth);
    };

    // --- Quantum Numbers ---
    if (!compact) { ImGui::TextDisabled("Quantum Numbers"); ImGui::Spacing(); }

    label("n (principal)");
    changed |= ImGui::SliderInt("##AtomN", &data->n, 1, 7);

    label("l (angular)");
    changed |= ImGui::SliderInt("##AtomL", &data->l, 0, data->n - 1);

    label("m (magnetic)");
    changed |= ImGui::SliderInt("##AtomM", &data->m, -data->l, data->l);

    if (!compact) { ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); }

    label("Scale");
    changed |= ImGui::SliderFloat("##BohrScale", &data->bohrScale, 0.1f, 10.0f, "%.2f");

    // --- Presets ---
    if (!compact) { ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); }
    if (!compact) ImGui::TextDisabled("Presets");
    ImGui::Spacing();

    auto preset = [&](const char* label, int pn, int pl, int pm) {
        if (ImGui::Button(label, ImVec2(compact ? 65.0f : 80.0f, 0)))
        {
            data->n = pn; data->l = pl; data->m = pm;
            changed = true;
        }
    };

    // Row 1: s-orbitals
    preset("1s", 1, 0, 0); ImGui::SameLine();
    preset("2s", 2, 0, 0); ImGui::SameLine();
    preset("3s", 3, 0, 0);

    // Row 2: p-orbitals
    preset("2p0", 2, 1, 0); ImGui::SameLine();
    preset("2p1", 2, 1, 1); ImGui::SameLine();
    preset("3p0", 3, 1, 0);

    // Row 3: d-orbitals
    preset("3d0", 3, 2, 0); ImGui::SameLine();
    preset("3d1", 3, 2, 1); ImGui::SameLine();
    preset("3d2", 3, 2, 2);

    // Row 4: f-orbitals and higher
    preset("4f0", 4, 3, 0); ImGui::SameLine();
    preset("4f1", 4, 3, 1); ImGui::SameLine();
    preset("4f3", 4, 3, 3);

    // Row 5: exotic high-n
    preset("5d2", 5, 2, 2); ImGui::SameLine();
    preset("6f3", 6, 3, 3); ImGui::SameLine();
    preset("7g4", 7, 4, 4);

    if (changed)
    {
        data->validate();
        data->needsGPUUpdate = true;
    }
}
