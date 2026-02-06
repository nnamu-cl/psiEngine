#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::projectionMatrix(float aspectRatio) const
{
    // GLM perspective assumes OpenGL conventions (NDC Y [-1,1], depth [-1,1]).
    // Vulkan NDC has Y flipped and depth [0,1].  Two corrections are applied
    // after the standard glm::perspective call.
    glm::mat4 proj = glm::perspective(
        glm::radians(fov),
        aspectRatio,
        nearPlane,
        farPlane
    );

    // Flip Y: Vulkan's framebuffer origin is top-left.
    proj[1][1] = -proj[1][1];

    // Remap depth from [-1,1] (OpenGL) to [0,1] (Vulkan).
    // z_vulkan = (z_opengl + 1) / 2, which in clip space becomes:
    //   z_clip_new = z_clip_old / 2 + w_clip / 2
    // w_clip = P[2][3]*z_view, so the per-element rewrite is:
    //   P[2][2] = (P[2][2] + P[2][3]) / 2    (P[2][3] is -1; folds in the +1)
    //   P[3][2] = P[3][2] / 2
    proj[2][2] = (proj[2][2] + proj[2][3]) / 2.0f;
    proj[3][2] = proj[3][2] / 2.0f;

    return proj;
}
