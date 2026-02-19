#pragma once

class PsiWorldLayer;
class PsiNodeEditorLayer;

/**
 * CreateToolbar - Floating "+" button that opens a context menu for adding
 * meshes and nodes to the world scene.
 *
 * Styled identically to GizmoToolbar (transparent background, icon button).
 * Positioned above the GizmoToolbar in the top-left corner of the viewport.
 */
class CreateToolbar
{
public:
    CreateToolbar(PsiWorldLayer* worldLayer, PsiNodeEditorLayer* nodeEditorLayer);

    void Render();

private:
    PsiWorldLayer*      m_WorldLayer;
    PsiNodeEditorLayer* m_NodeEditorLayer;

    void RenderMeshMenu();
    void RenderNodeMenu();
};
