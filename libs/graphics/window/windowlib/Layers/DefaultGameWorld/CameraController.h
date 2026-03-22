#pragma once

#include <SDL3/SDL.h>
#include <glm/vec3.hpp>

class Camera;

class CameraController
{
public:
    enum class Mode { None, Fly, Orbit, Pan };

    bool dynamicMainCamera = true;

    void init(Camera* camera, SDL_Window* window);
    void update(float ts);
    void onEvent(const SDL_Event& e);

    Mode activeMode() const { return m_ActiveMode; }

private:
    Camera*     m_Camera = nullptr;
    SDL_Window* m_Window = nullptr;

    // View angles in degrees
    float m_Yaw   = -90.0f;
    float m_Pitch =   0.0f;

    // Fly speed in world units/sec — adjusted permanently via scroll in fly mode
    float m_FlySpeed = 5.0f;

    // Orbit / pan focal point
    glm::vec3 m_FocalPoint{0.0f};
    float     m_FocalDistance = 5.0f;

    // Scroll accumulated from SDL events this frame, consumed in update()
    float m_ScrollDelta = 0.0f;

    // Tracks whether SDL relative mouse mode is currently active
    bool m_InRelativeMode = false;

    // Detects the first frame of orbit so the focal point can be initialised
    bool m_WasOrbiting = false;

    // Current active camera mode for UI display
    Mode m_ActiveMode = Mode::None;

    // Last camera state written by this controller.
    // If the camera differs from these at the start of update(), something external
    // (e.g. ViewManipulatorPanel) moved it, and we must re-sync yaw/pitch.
    glm::vec3 m_LastPosition{0.0f};
    glm::vec3 m_LastTarget{0.0f};

    void      syncAnglesFromCamera();
    glm::vec3 forwardDir() const;
    glm::vec3 rightDir()   const;
};
