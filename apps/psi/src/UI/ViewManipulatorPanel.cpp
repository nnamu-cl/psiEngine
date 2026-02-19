#include "ViewManipulatorPanel.h"

#include <imgui.h>
#include "ImGuizmo.h"
#include "Layers/DefaultGameWorld/Camera.h"
#include <glm/gtc/type_ptr.hpp>

void ViewManipulatorPanel::Render()
{
    Camera* camera = Camera::GetMain();
    if (!camera)
        return;

    ImGuiIO& io = ImGui::GetIO();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    glm::mat4 view = camera->viewMatrix();
    float viewMatrix[16];
    memcpy(viewMatrix, glm::value_ptr(view), sizeof(float) * 16);

    const float camDistance = glm::length(camera->position - camera->target);

    // Save a copy so we can detect whether ImGuizmo actually changed the matrix.
    float viewBefore[16];
    memcpy(viewBefore, viewMatrix, sizeof(float) * 16);

    ImGuizmo::ViewManipulate(
        viewMatrix,
        camDistance,
        ImVec2(0, 0),
        ImVec2(128, 128),
        0x10101010
    );

    // ImGuizmo writes back every frame even when idle, introducing floating-point
    // noise that would falsely trigger CameraController's external-change detection.
    // Only update the camera when the matrix actually changed.
    if (memcmp(viewBefore, viewMatrix, sizeof(float) * 16) == 0)
        return;

    // Reconstruct camera position and up from the (possibly modified) view matrix.
    // Inverting the view matrix gives us the camera's world-space transform:
    //   column 1 (index [1]) = world-space up
    //   column 3 (index [3]) = world-space position
    glm::mat4 newView = glm::make_mat4(viewMatrix);
    glm::mat4 invView  = glm::inverse(newView);
    camera->position   = glm::vec3(invView[3]);
    camera->up         = glm::vec3(invView[1]);
    // invView[2] is the world-space "back" vector; negate it to get forward.
    // Update target so the rest of the camera system stays consistent.
    camera->target = camera->position + (-glm::vec3(invView[2])) * camDistance;
}
