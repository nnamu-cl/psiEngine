#pragma once

/**
 * GizmoToolbar - Top-center toolbar for switching the active transform gizmo operation.
 *
 * Owns the current gizmo operation state. PsiUILayer reads it via GetOperation()
 * and passes it to ImGuizmo::Manipulate.
 *
 * Operation values match ImGuizmo::OPERATION (stored as int to avoid requiring
 * imgui.h/ImGuizmo.h in this header):
 *   Translate = 7, Rotate = 120, Scale = 896
 */
class GizmoToolbar
{
public:
    void Render();

    int GetOperation() const { return m_Operation; }

private:
    int m_Operation = 7; // ImGuizmo::TRANSLATE
};
