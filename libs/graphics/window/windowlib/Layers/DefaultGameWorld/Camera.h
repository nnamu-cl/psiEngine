#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Camera
{
public:
    glm::vec3 position{ 0.0f, 0.0f, 5.0f };
    glm::vec3 target{   0.0f, 0.0f, 0.0f };
    glm::vec3 up{       0.0f, 1.0f, 0.0f };

    float fov       = 45.0f;
    float nearPlane = 0.1f;
    float farPlane  = 100.0f;

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix(float aspectRatio) const;
};
