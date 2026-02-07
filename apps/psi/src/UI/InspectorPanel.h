#pragma once

// Forward declarations
class PsiWorldLayer;
class PsiNodeEditorLayer;

/**
 * InspectorPanel - Object property inspector
 *
 * Positioned below the stats panel at top right.
 * Displays and allows editing of transform properties
 * (position, rotation, scale) for the currently selected object.
 */
class InspectorPanel
{
public:
    InspectorPanel(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer);

    void Render();

    bool IsVisible() const { return m_Visible; }
    void SetVisible(bool visible) { m_Visible = visible; }

private:
    PsiWorldLayer* m_WorldLayer;
    PsiNodeEditorLayer* m_NodeEditorLayer;
    bool m_Visible = true;
};
