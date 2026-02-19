//
// Tests for CameraController, GizmoToolbar state, and ViewManipulatorPanel math.
//
// GizmoToolbar:       tests default state only — Render() needs ImGui context.
// ViewManipulatorPanel: tests the inverse-view-matrix extraction math directly
//                       against Camera objects (no ImGui / ImGuizmo required).
// CameraController:   tests scroll zoom, the disabled guard, external-change
//                     re-sync (the handshake with ViewManipulatorPanel), and
//                     scroll accumulation.  Requires SDL_INIT_VIDEO / _EVENTS
//                     so CameraController::update() can query input state, but
//                     NO Vulkan context and NO real window are needed.
//

#include <catch2/catch_test_macros.hpp>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include "Layers/DefaultGameWorld/Camera.h"
#include "Layers/DefaultGameWorld/CameraController.h"
#include "UI/GizmoToolbar.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static constexpr float EPS = 1e-3f;

#define APPROX(value, target) REQUIRE(std::abs(static_cast<float>(value) - static_cast<float>(target)) <= EPS)

static bool vec3Approx(const glm::vec3& a, const glm::vec3& b, float tol = EPS)
{
    return glm::length(a - b) < tol;
}

// Must match the private constant in CameraController.cpp.
static constexpr float kScrollZoomStep = 0.50f;

// RAII SDL initializer.
// CameraController::update() calls SDL_GetKeyboardState / SDL_GetMouseState,
// which are no-ops until SDL is initialized.  No window is created here —
// SDL_SetWindowRelativeMouseMode is only reached when a mouse button is held,
// which never happens in these tests.
struct SDLGuard
{
    SDLGuard()  { SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS); }
    ~SDLGuard() { SDL_Quit(); }
};

// Helper: build a scroll wheel event with the given vertical delta.
static SDL_Event makeScrollEvent(float y)
{
    SDL_Event e{};
    e.type    = SDL_EVENT_MOUSE_WHEEL;
    e.wheel.y = y;
    return e;
}

// ---------------------------------------------------------------------------
// GizmoToolbar
// ---------------------------------------------------------------------------
TEST_CASE("GizmoToolbar: default operation is TRANSLATE (7)")
{
    // GetOperation() is inline and the constructor is implicit — neither
    // requires an ImGui context, so this test works without any rendering setup.
    GizmoToolbar toolbar;
    REQUIRE(toolbar.GetOperation() == 7);   // ImGuizmo::TRANSLATE == 7
}

// ---------------------------------------------------------------------------
// ViewManipulatorPanel math
//
// ViewManipulatorPanel.cpp lines 47-53 reconstruct camera position, up, and
// target by inverting the view matrix that ImGuizmo modifies.  We validate
// that exact math here using only Camera and glm — no ImGuizmo needed.
// ---------------------------------------------------------------------------
TEST_CASE("ViewManipulator math: inverting the view matrix recovers camera position")
{
    Camera cam;
    cam.position = {3.0f, 4.0f, 5.0f};
    cam.target   = {1.0f, 2.0f, 0.0f};
    cam.up       = {0.0f, 1.0f, 0.0f};

    glm::mat4 view    = cam.viewMatrix();
    glm::mat4 invView = glm::inverse(view);

    // Column 3 of the inverse view matrix is the camera world-space position
    // (corresponds to ViewManipulatorPanel.cpp line 49).
    glm::vec3 recoveredPos = glm::vec3(invView[3]);
    REQUIRE(vec3Approx(recoveredPos, cam.position));
}

TEST_CASE("ViewManipulator math: reconstructed target lies on the original look-at ray")
{
    Camera cam;
    cam.position = {3.0f, 4.0f, 5.0f};
    cam.target   = {1.0f, 2.0f, 0.0f};
    cam.up       = {0.0f, 1.0f, 0.0f};

    const float camDistance = glm::length(cam.position - cam.target);

    glm::mat4 view    = cam.viewMatrix();
    glm::mat4 invView = glm::inverse(view);

    // Column 2 of invView is the world-space "back" vector; negate for forward.
    // Corresponds to ViewManipulatorPanel.cpp line 53.
    glm::vec3 pos         = glm::vec3(invView[3]);
    glm::vec3 forward     = -glm::vec3(invView[2]);
    glm::vec3 reconTarget = pos + forward * camDistance;

    REQUIRE(vec3Approx(reconTarget, cam.target));
}

// ---------------------------------------------------------------------------
// CameraController
// ---------------------------------------------------------------------------

TEST_CASE("CameraController: scroll zoom moves camera and target along the forward axis")
{
    SDLGuard sdl;

    // Camera on the Z axis, looking at the origin.
    // syncAnglesFromCamera will derive: yaw = -90°, pitch = 0°
    // → forwardDir() = (0, 0, -1)
    Camera cam;
    cam.position = {0.0f, 0.0f, 5.0f};
    cam.target   = {0.0f, 0.0f, 0.0f};
    cam.up       = {0.0f, 1.0f, 0.0f};

    CameraController ctrl;
    ctrl.init(&cam, nullptr);   // null window: no mouse buttons held → never touched

    SDL_Event e = makeScrollEvent(1.0f);
    ctrl.onEvent(e);
    ctrl.update(0.016f);

    // delta = forward(0,0,-1) * 1.0 * kScrollZoomStep(0.5) = (0, 0, -0.5)
    APPROX(cam.position.x,  0.0f);
    APPROX(cam.position.y,  0.0f);
    APPROX(cam.position.z,  5.0f - kScrollZoomStep);

    APPROX(cam.target.x,  0.0f);
    APPROX(cam.target.y,  0.0f);
    APPROX(cam.target.z,  0.0f - kScrollZoomStep);
}

TEST_CASE("CameraController: dynamicMainCamera=false prevents all camera movement")
{
    SDLGuard sdl;

    Camera cam;
    cam.position = {0.0f, 0.0f, 5.0f};
    cam.target   = {0.0f, 0.0f, 0.0f};
    cam.up       = {0.0f, 1.0f, 0.0f};

    CameraController ctrl;
    ctrl.init(&cam, nullptr);
    ctrl.dynamicMainCamera = false;

    // Large scroll that would definitely move the camera if the guard were absent.
    SDL_Event e = makeScrollEvent(10.0f);
    ctrl.onEvent(e);
    ctrl.update(0.016f);

    APPROX(cam.position.z,  5.0f);
    APPROX(cam.target.z,    0.0f);
}

TEST_CASE("CameraController: external camera move is detected and angles re-sync correctly")
{
    SDLGuard sdl;

    // This test validates the handshake between ViewManipulatorPanel (which
    // writes new position/target directly to the camera) and CameraController
    // (which must detect the external change and re-derive its internal yaw/pitch).

    // Start on the Z axis.
    Camera cam;
    cam.position = {0.0f, 0.0f, 5.0f};
    cam.target   = {0.0f, 0.0f, 0.0f};
    cam.up       = {0.0f, 1.0f, 0.0f};

    CameraController ctrl;
    ctrl.init(&cam, nullptr);

    // First update with no input: records m_LastPosition=(0,0,5), m_LastTarget=(0,0,0).
    ctrl.update(0.016f);

    // Simulate ViewManipulatorPanel snapping the camera to the +X axis.
    cam.position = {5.0f, 0.0f, 0.0f};
    cam.target   = {0.0f, 0.0f, 0.0f};
    cam.up       = {0.0f, 1.0f, 0.0f};

    // Second update with no input: controller sees position differs from
    // m_LastPosition and calls syncAnglesFromCamera().
    // New dir = (-1,0,0) → yaw = 180°, pitch = 0° → forwardDir() = (-1, 0, 0).
    ctrl.update(0.016f);

    // Inject scroll +1 and update again.
    // If re-sync happened: moves along new forward (-1, 0, 0).
    // If re-sync was skipped: would wrongly move along old forward (0, 0, -1).
    SDL_Event e = makeScrollEvent(1.0f);
    ctrl.onEvent(e);
    ctrl.update(0.016f);

    // delta = (-1,0,0) * 1.0 * 0.5 = (-0.5, 0, 0)
    APPROX(cam.position.x,  5.0f - kScrollZoomStep);
    APPROX(cam.position.y,  0.0f);
    APPROX(cam.position.z,  0.0f);
}

TEST_CASE("CameraController: multiple scroll events in one frame accumulate correctly")
{
    SDLGuard sdl;

    Camera cam;
    cam.position = {0.0f, 0.0f, 5.0f};
    cam.target   = {0.0f, 0.0f, 0.0f};
    cam.up       = {0.0f, 1.0f, 0.0f};

    CameraController ctrl;
    ctrl.init(&cam, nullptr);

    // Three separate scroll events before a single update call.
    for (int i = 0; i < 3; ++i)
    {
        SDL_Event e = makeScrollEvent(1.0f);
        ctrl.onEvent(e);
    }

    ctrl.update(0.016f);

    // m_ScrollDelta accumulated to 3.0 → delta = (0,0,-1) * 3 * 0.5 = (0,0,-1.5)
    APPROX(cam.position.z,  5.0f - 3.0f * kScrollZoomStep);
    APPROX(cam.target.z,    0.0f - 3.0f * kScrollZoomStep);
}
