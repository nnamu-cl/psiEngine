#include "CameraController.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <cmath>

// Mouse sensitivity (degrees per pixel)
static constexpr float kSensitivity    = 0.20f;
// Speed multiplier while Shift is held in fly mode
static constexpr float kShiftMultiplier = 3.0f;
// Each scroll notch multiplies fly speed by (1 +/- this fraction)
static constexpr float kScrollFlyStep  = 0.10f;
// World-unit dolly/zoom per scroll notch
static constexpr float kScrollZoomStep = 0.50f;
// Pan units per pixel, scaled by focal distance so it feels consistent
static constexpr float kPanScale       = 0.005f;

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void CameraController::init(Camera* camera, SDL_Window* window)
{
    m_Camera = camera;
    m_Window = window;
    syncAnglesFromCamera();
}

void CameraController::onEvent(const SDL_Event& e)
{
    if (!dynamicMainCamera)
        return;

    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (e.type == SDL_EVENT_MOUSE_WHEEL)
        m_ScrollDelta += e.wheel.y;
}

void CameraController::update(float ts)
{
    if (!m_Camera || !dynamicMainCamera)
        return;

    // Re-sync internal yaw/pitch if an external system (e.g. ViewManipulatorPanel)
    // moved the camera since we last wrote it.  We use a small epsilon to ignore
    // floating-point noise from view-matrix round-trips.
    static constexpr float kExternalMovedEps = 1e-4f;
    if (glm::length(m_Camera->position - m_LastPosition) > kExternalMovedEps ||
        glm::length(m_Camera->target   - m_LastTarget)   > kExternalMovedEps)
    {
        syncAnglesFromCamera();
    }

    // --- Query current input state ---
    const bool*          keys    = SDL_GetKeyboardState(nullptr);
    const SDL_Keymod     mods    = SDL_GetModState();
    float                mx, my;
    SDL_MouseButtonFlags buttons = SDL_GetMouseState(&mx, &my);

    const ImGuiIO& io             = ImGui::GetIO();
    const bool     mouseBlocked   = io.WantCaptureMouse;
    const bool     keyboardBlocked = io.WantCaptureKeyboard;

    const bool rmb       = !mouseBlocked    && (buttons & SDL_BUTTON_RMASK) != 0;
    const bool lmb       = !mouseBlocked    && (buttons & SDL_BUTTON_LMASK) != 0;
    const bool altHeld   = !keyboardBlocked && (mods & SDL_KMOD_ALT)        != 0;
    const bool shiftHeld = !keyboardBlocked && (mods & SDL_KMOD_SHIFT)      != 0;

    // Determine which mode is active (priority: fly > orbit > shift-pan)
    // Shift+RMB = pan, plain RMB = fly
    const bool wantPan    = shiftHeld && rmb && !altHeld;
    const bool wantFly    = rmb && !shiftHeld;
    const bool wantOrbit  = altHeld && lmb && !rmb;

    const bool wantRelative = wantFly || wantOrbit || wantPan;

    // Toggle SDL relative mouse mode — this also hides/shows the cursor.
    // On the frame we enter relative mode, SDL_GetRelativeMouseState returns
    // a stale accumulated delta that would cause a camera jump, so we flush
    // that first reading and use zero for this frame's delta.
    bool justEnteredRelative = false;
    if (wantRelative != m_InRelativeMode)
    {
        SDL_SetWindowRelativeMouseMode(m_Window, wantRelative);
        m_InRelativeMode = wantRelative;
        if (wantRelative)
            justEnteredRelative = true;
    }

    float dx = 0.0f, dy = 0.0f;
    if (wantRelative)
    {
        SDL_GetRelativeMouseState(&dx, &dy);
        if (justEnteredRelative)
        {
            dx = 0.0f;
            dy = 0.0f;
        }
    }

    // --- Orbit entry: place focal point in front of the camera ---
    // On the first frame of orbit we pick a sensible focal point rather than
    // trusting whatever target the camera happened to have (e.g. from fly mode).
    if (wantOrbit && !m_WasOrbiting)
    {
        // Clamp focal distance so entering orbit after a close-up dolly or fly
        // mode doesn't place the pivot unreasonably near the camera.
        m_FocalDistance  = glm::max(m_FocalDistance, 1.0f);
        m_FocalPoint     = m_Camera->position + forwardDir() * m_FocalDistance;
        m_Camera->target = m_FocalPoint;
    }
    m_WasOrbiting = wantOrbit;

    // Track active mode for UI hints
    if (wantFly)        m_ActiveMode = Mode::Fly;
    else if (wantOrbit) m_ActiveMode = Mode::Orbit;
    else if (wantPan)   m_ActiveMode = Mode::Pan;
    else                m_ActiveMode = Mode::None;

    // -----------------------------------------------------------------------
    // Fly mode  (RMB held)
    // -----------------------------------------------------------------------
    if (wantFly)
    {
        // Mouse look
        m_Yaw   += dx * kSensitivity;
        m_Pitch -= dy * kSensitivity;
        m_Pitch  = glm::clamp(m_Pitch, -89.0f, 89.0f);

        // WASD + Q/E movement
        const glm::vec3 fwd   = forwardDir();
        const glm::vec3 right = rightDir();
        const bool shifting   = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
        const float speed     = m_FlySpeed * (shifting ? kShiftMultiplier : 1.0f);

        glm::vec3 move{0.0f};
        if (keys[SDL_SCANCODE_W]) move += fwd;
        if (keys[SDL_SCANCODE_S]) move -= fwd;
        if (keys[SDL_SCANCODE_A]) move -= right;
        if (keys[SDL_SCANCODE_D]) move += right;
        if (keys[SDL_SCANCODE_E]) move += m_Camera->up;
        if (keys[SDL_SCANCODE_Q]) move -= m_Camera->up;

        if (glm::length(move) > 0.0f)
            m_Camera->position += glm::normalize(move) * speed * ts;

        // Keep target 1 unit ahead so viewMatrix() stays valid
        m_Camera->target = m_Camera->position + forwardDir();

        // Scroll adjusts fly speed permanently
        if (m_ScrollDelta != 0.0f)
        {
            m_FlySpeed *= std::pow(1.0f + kScrollFlyStep, m_ScrollDelta);
            m_FlySpeed  = glm::clamp(m_FlySpeed, 0.5f, 100.0f);
        }
    }
    // -----------------------------------------------------------------------
    // Orbit mode  (Alt + LMB)
    // -----------------------------------------------------------------------
    else if (wantOrbit)
    {
        m_Yaw   += dx * kSensitivity;
        m_Pitch -= dy * kSensitivity;
        m_Pitch  = glm::clamp(m_Pitch, -89.0f, 89.0f);

        // Scroll: dolly in/out along the view axis
        if (m_ScrollDelta != 0.0f)
        {
            m_FocalDistance -= m_ScrollDelta * kScrollZoomStep;
            m_FocalDistance  = glm::max(0.1f, m_FocalDistance);
        }

        // Camera always sits behind the focal point along the current view direction
        m_Camera->position = m_FocalPoint + (-forwardDir()) * m_FocalDistance;
        m_Camera->target   = m_FocalPoint;
    }
    // -----------------------------------------------------------------------
    // Shift + RMB: pan camera and focal point together
    // -----------------------------------------------------------------------
    else if (wantPan)
    {
        const glm::vec3 right = rightDir();
        const float     scale = kPanScale * m_FocalDistance;
        const glm::vec3 delta = (-right * dx + m_Camera->up * dy) * scale;

        m_FocalPoint       += delta;
        m_Camera->position += delta;
        m_Camera->target   += delta;
    }
    // -----------------------------------------------------------------------
    // Scroll only — zoom along the view axis
    // -----------------------------------------------------------------------
    else if (m_ScrollDelta != 0.0f)
    {
        const glm::vec3 delta = forwardDir() * m_ScrollDelta * kScrollZoomStep;
        m_Camera->position   += delta;
        m_Camera->target     += delta;
    }

    // F key: focus selected (TODO — no selection system yet)

    m_ScrollDelta = 0.0f;

    // Record what we wrote so we can detect external changes next frame.
    m_LastPosition = m_Camera->position;
    m_LastTarget   = m_Camera->target;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

glm::vec3 CameraController::forwardDir() const
{
    const float yr = glm::radians(m_Yaw);
    const float pr = glm::radians(m_Pitch);
    return glm::normalize(glm::vec3{
        std::cos(yr) * std::cos(pr),
        std::sin(pr),
        std::sin(yr) * std::cos(pr)
    });
}

glm::vec3 CameraController::rightDir() const
{
    return glm::normalize(glm::cross(forwardDir(), m_Camera->up));
}

void CameraController::syncAnglesFromCamera()
{
    const glm::vec3 dir = glm::normalize(m_Camera->target - m_Camera->position);
    m_Pitch             = glm::degrees(std::asin(glm::clamp(dir.y, -1.0f, 1.0f)));
    m_Yaw               = glm::degrees(std::atan2(dir.z, dir.x));
    m_FocalPoint        = m_Camera->target;
    m_FocalDistance     = glm::length(m_Camera->target - m_Camera->position);

    // Reset up to world-up to prevent drift caused by gizmo snaps writing a
    // non-standard up vector (e.g. top-down snap sets up = (0,0,-1)).
    // If pitch is near ±90° the forward and world-up are nearly parallel, so
    // we derive up from a cross with the world-right axis instead.
    static const glm::vec3 kWorldUp{0.0f, 1.0f, 0.0f};
    if (std::abs(m_Pitch) < 80.0f)
    {
        m_Camera->up = kWorldUp;
    }
    else
    {
        const glm::vec3 worldRight{1.0f, 0.0f, 0.0f};
        const glm::vec3 candidate = glm::cross(dir, worldRight);
        m_Camera->up = (glm::length(candidate) > 0.001f)
                           ? glm::normalize(candidate)
                           : kWorldUp;
    }

    // Keep last-known state in sync so update() doesn't immediately re-trigger.
    m_LastPosition = m_Camera->position;
    m_LastTarget   = m_Camera->target;
}
