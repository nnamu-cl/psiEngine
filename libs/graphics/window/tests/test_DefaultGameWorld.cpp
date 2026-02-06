//
// Unit tests for Camera, Transform/Scene, Mesh, and MeshTable.
// All tests are pure CPU -- no Vulkan context required.
//

#include <catch2/catch_test_macros.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include "Layers/DefaultGameWorld/Camera.h"
#include "Layers/DefaultGameWorld/Scene.h"
#include "Layers/DefaultGameWorld/Mesh.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static constexpr float EPS = 1e-4f;

// Scalar float-tolerance check.  WithinAbsMatcher only matches double, so we
// use a plain macro to keep every REQUIRE_THAT call below compiling cleanly.
#define APPROX(value, target) REQUIRE(std::abs((value) - (target)) <= EPS)

// Column-major element access: glm::mat4[col][row].
// Compare two mat4s element-wise within a tolerance.
static bool matApprox(const glm::mat4& a, const glm::mat4& b, float tol = EPS)
{
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            if (std::abs(a[c][r] - b[c][r]) > tol)
                return false;
    return true;
}

// ---------------------------------------------------------------------------
// Camera -- viewMatrix
// ---------------------------------------------------------------------------
TEST_CASE("Camera viewMatrix produces identity-like result for default state")
{
    // Default camera: pos(0,0,5) looking at origin, up=(0,1,0).
    // The view matrix should translate -5 along Z (into screen).
    Camera cam;
    glm::mat4 V = cam.viewMatrix();

    // The translation column (col 3) should be (0, 0, -5).
    APPROX(V[3][0],  0.0f);
    APPROX(V[3][1],  0.0f);
    APPROX(V[3][2], -5.0f);
}

TEST_CASE("Camera viewMatrix matches glm::lookAt")
{
    Camera cam;
    cam.position = { 3.0f, 4.0f, 5.0f };
    cam.target   = { 1.0f, 2.0f, 0.0f };
    cam.up       = { 0.0f, 1.0f, 0.0f };

    glm::mat4 expected = glm::lookAt(cam.position, cam.target, cam.up);
    REQUIRE(matApprox(cam.viewMatrix(), expected));
}

TEST_CASE("Camera viewMatrix changes when position moves")
{
    Camera cam;
    glm::mat4 v1 = cam.viewMatrix();

    cam.position = { 10.0f, 0.0f, 0.0f };
    glm::mat4 v2 = cam.viewMatrix();

    REQUIRE(!matApprox(v1, v2));
}

// ---------------------------------------------------------------------------
// Camera -- projectionMatrix
// ---------------------------------------------------------------------------
TEST_CASE("Camera projectionMatrix has correct structure")
{
    Camera cam;
    cam.fov       = 90.0f;
    cam.nearPlane = 0.1f;
    cam.farPlane  = 100.0f;

    glm::mat4 P = cam.projectionMatrix(1.0f);  // square viewport

    // For a 90-degree FOV with aspect 1, the top-left 2x2 should be
    // diagonal with |value| == 1.  The [1][1] is negated (Vulkan flip).
    APPROX(std::abs(P[0][0]),  1.0f);
    APPROX(P[1][1], -1.0f);   // negated for Vulkan Y-flip

    // [2][3] must be -1 (perspective divide indicator).
    APPROX(P[2][3], -1.0f);

    // Near-plane maps to depth 0 after Vulkan correction.
    // depth_at_near = (far*near) / (far - near) ... simplified check:
    // [3][2] should be non-zero (translation part of depth).
    REQUIRE(std::abs(P[3][2]) > EPS);
}

TEST_CASE("Camera projectionMatrix depth range maps near to 0, far to 1")
{
    Camera cam;
    cam.fov       = 60.0f;
    cam.nearPlane = 1.0f;
    cam.farPlane  = 100.0f;

    glm::mat4 P = cam.projectionMatrix(1.6f);

    // A point exactly at the near plane in view space: (0, 0, -nearPlane, 1).
    glm::vec4 nearPt = P * glm::vec4(0.0f, 0.0f, -cam.nearPlane, 1.0f);
    float nearDepth  = nearPt.z / nearPt.w;   // perspective divide
    APPROX(nearDepth, 0.0f);

    // A point exactly at the far plane.
    glm::vec4 farPt  = P * glm::vec4(0.0f, 0.0f, -cam.farPlane, 1.0f);
    float farDepth   = farPt.z / farPt.w;
    APPROX(farDepth, 1.0f);
}

TEST_CASE("Camera projectionMatrix aspect ratio scales X correctly")
{
    Camera cam;
    cam.fov = 90.0f;

    glm::mat4 P1 = cam.projectionMatrix(1.0f);   // square
    glm::mat4 P2 = cam.projectionMatrix(2.0f);   // wide

    // [0][0] should be halved when aspect doubles.
    APPROX(P2[0][0], P1[0][0] / 2.0f);

    // [1][1] (Y scale) should NOT change with aspect.
    APPROX(P2[1][1], P1[1][1]);
}

// ---------------------------------------------------------------------------
// Transform -- toMatrix
// ---------------------------------------------------------------------------
TEST_CASE("Transform toMatrix is identity for default values")
{
    Transform t;   // pos=0, rot=identity quat, scale=1
    REQUIRE(matApprox(t.toMatrix(), glm::mat4(1.0f)));
}

TEST_CASE("Transform toMatrix applies translation only")
{
    Transform t;
    t.position = { 3.0f, -2.0f, 7.0f };

    glm::mat4 expected = glm::translate(glm::mat4(1.0f), t.position);
    REQUIRE(matApprox(t.toMatrix(), expected));
}

TEST_CASE("Transform toMatrix applies uniform scale only")
{
    Transform t;
    t.scale = { 2.0f, 2.0f, 2.0f };

    glm::mat4 expected = glm::scale(glm::mat4(1.0f), t.scale);
    REQUIRE(matApprox(t.toMatrix(), expected));
}

TEST_CASE("Transform toMatrix applies non-uniform scale")
{
    Transform t;
    t.scale = { 1.0f, 3.0f, 0.5f };

    glm::mat4 expected = glm::scale(glm::mat4(1.0f), t.scale);
    REQUIRE(matApprox(t.toMatrix(), expected));
}

TEST_CASE("Transform toMatrix applies 90-degree rotation around Y")
{
    Transform t;
    // 90 degrees around Y axis.
    t.rotation = glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f));

    glm::mat4 expected = glm::mat4_cast(t.rotation);
    REQUIRE(matApprox(t.toMatrix(), expected));
}

TEST_CASE("Transform toMatrix composes TRS in correct order")
{
    Transform t;
    t.position = { 1.0f, 2.0f, 3.0f };
    t.rotation = glm::quat(glm::vec3(0.0f, glm::radians(45.0f), 0.0f));
    t.scale    = { 2.0f, 1.0f, 2.0f };

    // Expected: T * R * S  (column-major, applied right-to-left to a point)
    glm::mat4 expected = glm::translate(glm::mat4(1.0f), t.position);
    expected            = expected * glm::mat4_cast(t.rotation);
    expected            = glm::scale(expected, t.scale);

    REQUIRE(matApprox(t.toMatrix(), expected));
}

TEST_CASE("Transform toMatrix transforms a known point correctly")
{
    Transform t;
    t.position = { 10.0f, 0.0f, 0.0f };
    t.scale    = { 2.0f, 2.0f, 2.0f };
    // No rotation.

    glm::mat4 M = t.toMatrix();
    // Point (1,0,0) -> scaled to (2,0,0) -> translated to (12,0,0).
    glm::vec4 result = M * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    APPROX(result.x, 12.0f);
    APPROX(result.y,  0.0f);
    APPROX(result.z,  0.0f);
}

// ---------------------------------------------------------------------------
// Scene -- addObject / clear
// ---------------------------------------------------------------------------
TEST_CASE("Scene starts empty")
{
    Scene s;
    REQUIRE(s.objects.empty());
}

TEST_CASE("Scene addObject increases count")
{
    Scene s;
    GameObject obj{};
    s.addObject(obj);
    REQUIRE(s.objects.size() == 1);

    s.addObject(obj);
    REQUIRE(s.objects.size() == 2);
}

TEST_CASE("Scene addObject preserves data")
{
    Scene s;
    GameObject obj{};
    obj.meshIndex     = 5;
    obj.materialIndex = 3;
    obj.transform.position = { 1.0f, 2.0f, 3.0f };

    s.addObject(obj);

    REQUIRE(s.objects[0].meshIndex     == 5);
    REQUIRE(s.objects[0].materialIndex == 3);
    APPROX(s.objects[0].transform.position.x, 1.0f);
}

TEST_CASE("Scene clear empties the object list")
{
    Scene s;
    GameObject obj{};
    s.addObject(obj);
    s.addObject(obj);
    s.addObject(obj);
    REQUIRE(s.objects.size() == 3);

    s.clear();
    REQUIRE(s.objects.empty());
}

TEST_CASE("Scene clear on an already-empty scene is a no-op")
{
    Scene s;
    s.clear();
    REQUIRE(s.objects.empty());
}

TEST_CASE("Scene with empty objects can be safely iterated")
{
    Scene s;
    // Iterate over empty scene - should not crash
    int count = 0;
    for (const auto& obj : s.objects) {
        (void)obj;  // Suppress unused warning
        count++;
    }
    REQUIRE(count == 0);
}

TEST_CASE("Scene objects can be accessed after clearing")
{
    Scene s;
    GameObject obj{};
    s.addObject(obj);
    s.addObject(obj);
    REQUIRE(s.objects.size() == 2);

    s.clear();
    REQUIRE(s.objects.size() == 0);

    // After clear, we should be able to add objects again
    s.addObject(obj);
    REQUIRE(s.objects.size() == 1);
}

// ---------------------------------------------------------------------------
// MeshTable -- add / get / count
// ---------------------------------------------------------------------------
TEST_CASE("MeshTable starts empty")
{
    MeshTable table;
    REQUIRE(table.count() == 0);
}

TEST_CASE("MeshTable add returns sequential indices")
{
    MeshTable table;
    Mesh m{};
    uint32_t i0 = table.add("a", m);
    uint32_t i1 = table.add("b", m);
    uint32_t i2 = table.add("c", m);

    REQUIRE(i0 == 0);
    REQUIRE(i1 == 1);
    REQUIRE(i2 == 2);
    REQUIRE(table.count() == 3);
}

TEST_CASE("MeshTable add with duplicate name replaces and returns same index")
{
    MeshTable table;

    Mesh m1{};
    m1.vertices.push_back({ glm::vec3(1,0,0), glm::vec3(0,0,1), glm::vec2(0,0) });
    uint32_t idx = table.add("cube", m1);

    Mesh m2{};
    m2.vertices.push_back({ glm::vec3(9,9,9), glm::vec3(0,0,1), glm::vec2(0,0) });
    m2.vertices.push_back({ glm::vec3(8,8,8), glm::vec3(0,0,1), glm::vec2(0,0) });
    uint32_t idx2 = table.add("cube", m2);

    REQUIRE(idx2 == idx);
    REQUIRE(table.count() == 1);

    const Mesh* got = table.get(idx);
    REQUIRE(got != nullptr);
    REQUIRE(got->vertices.size() == 2);
    APPROX(got->vertices[0].position.x, 9.0f);
}

TEST_CASE("MeshTable get by index returns nullptr for out-of-range")
{
    MeshTable table;
    REQUIRE(table.get(0u) == nullptr);
    REQUIRE(table.get(99u) == nullptr);
}

TEST_CASE("MeshTable get by name returns nullptr for unknown name")
{
    MeshTable table;
    REQUIRE(table.get("missing") == nullptr);
}

TEST_CASE("MeshTable get by name finds added mesh")
{
    MeshTable table;
    Mesh m{};
    m.indices = { 0, 1, 2 };
    table.add("tri", m);

    const Mesh* got = table.get("tri");
    REQUIRE(got != nullptr);
    REQUIRE(got->indices.size() == 3);
}

// ---------------------------------------------------------------------------
// MeshTable::unitTriangle
// ---------------------------------------------------------------------------
TEST_CASE("unitTriangle has correct vertex and index counts")
{
    Mesh tri = MeshTable::unitTriangle();
    REQUIRE(tri.vertices.size() == 3);
    REQUIRE(tri.indices.size()  == 3);
}

TEST_CASE("unitTriangle indices reference valid vertices")
{
    Mesh tri = MeshTable::unitTriangle();
    for (auto idx : tri.indices)
        REQUIRE(idx < tri.vertices.size());
}

TEST_CASE("unitTriangle normals all point +Z")
{
    Mesh tri = MeshTable::unitTriangle();
    for (auto& v : tri.vertices)
    {
        APPROX(v.normal.x, 0.0f);
        APPROX(v.normal.y, 0.0f);
        APPROX(v.normal.z, 1.0f);
    }
}

TEST_CASE("unitTriangle lies in the Z=0 plane")
{
    Mesh tri = MeshTable::unitTriangle();
    for (auto& v : tri.vertices)
        APPROX(v.position.z, 0.0f);
}

// ---------------------------------------------------------------------------
// MeshTable::unitCube
// ---------------------------------------------------------------------------
TEST_CASE("unitCube has correct vertex and index counts")
{
    Mesh cube = MeshTable::unitCube();
    // 6 faces x 4 verts = 24, 6 faces x 6 indices = 36
    REQUIRE(cube.vertices.size() == 24);
    REQUIRE(cube.indices.size()  == 36);
}

TEST_CASE("unitCube indices reference valid vertices")
{
    Mesh cube = MeshTable::unitCube();
    for (auto idx : cube.indices)
        REQUIRE(idx < cube.vertices.size());
}

TEST_CASE("unitCube all vertices are within [-1,1] on each axis")
{
    Mesh cube = MeshTable::unitCube();
    for (auto& v : cube.vertices)
    {
        REQUIRE(std::abs(v.position.x) <= 1.0f + EPS);
        REQUIRE(std::abs(v.position.y) <= 1.0f + EPS);
        REQUIRE(std::abs(v.position.z) <= 1.0f + EPS);
    }
}

TEST_CASE("unitCube normals are axis-aligned unit vectors")
{
    Mesh cube = MeshTable::unitCube();
    for (auto& v : cube.vertices)
    {
        float len = glm::length(v.normal);
        APPROX(len, 1.0f);

        // Exactly one component is +/-1, the other two are 0.
        int nonZero = 0;
        if (std::abs(v.normal.x) > EPS) ++nonZero;
        if (std::abs(v.normal.y) > EPS) ++nonZero;
        if (std::abs(v.normal.z) > EPS) ++nonZero;
        REQUIRE(nonZero == 1);
    }
}

TEST_CASE("unitCube has exactly 6 distinct normals (one per face)")
{
    Mesh cube = MeshTable::unitCube();
    std::vector<glm::vec3> unique;
    for (auto& v : cube.vertices)
    {
        bool found = false;
        for (auto& u : unique)
            if (glm::length(u - v.normal) < EPS) { found = true; break; }
        if (!found) unique.push_back(v.normal);
    }
    REQUIRE(unique.size() == 6);
}

TEST_CASE("unitCube texCoords are within [0,1]")
{
    Mesh cube = MeshTable::unitCube();
    for (auto& v : cube.vertices)
    {
        REQUIRE(v.texCoord.x >= -EPS);
        REQUIRE(v.texCoord.x <=  1.0f + EPS);
        REQUIRE(v.texCoord.y >= -EPS);
        REQUIRE(v.texCoord.y <=  1.0f + EPS);
    }
}

// ---------------------------------------------------------------------------
// Integration: Transform + Mesh vertex transform
// ---------------------------------------------------------------------------
TEST_CASE("Applying a Transform matrix to all cube vertices moves the cube")
{
    Transform t;
    t.position = { 5.0f, 0.0f, 0.0f };
    glm::mat4 M = t.toMatrix();

    Mesh cube = MeshTable::unitCube();
    for (auto& v : cube.vertices)
    {
        glm::vec4 worldPos = M * glm::vec4(v.position, 1.0f);
        // All X coords should now be in [4, 6].
        REQUIRE(worldPos.x >= 4.0f - EPS);
        REQUIRE(worldPos.x <= 6.0f + EPS);
    }
}

TEST_CASE("Transform with rotation affects vertex positions correctly")
{
    Transform t;
    // 90 degree rotation around Y axis: X becomes Z, Z becomes -X
    t.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 M = t.toMatrix();

    // Point at (1, 0, 0) should rotate to approximately (0, 0, -1)
    glm::vec4 point = M * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    APPROX(point.x,  0.0f);
    APPROX(point.y,  0.0f);
    APPROX(point.z, -1.0f);
    APPROX(point.w,  1.0f);
}

TEST_CASE("Transform with scale affects vertex positions correctly")
{
    Transform t;
    t.scale = glm::vec3(2.0f, 3.0f, 4.0f);
    glm::mat4 M = t.toMatrix();

    // Point at (1, 1, 1) should scale to (2, 3, 4)
    glm::vec4 point = M * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    APPROX(point.x, 2.0f);
    APPROX(point.y, 3.0f);
    APPROX(point.z, 4.0f);
    APPROX(point.w, 1.0f);
}

TEST_CASE("Transform order matters: TRS produces different results than SRT")
{
    // Transform 1: Translate then scale (our standard TRS order)
    Transform t1;
    t1.position = glm::vec3(1.0f, 0.0f, 0.0f);
    t1.scale = glm::vec3(2.0f);
    glm::mat4 M1 = t1.toMatrix();

    // Transform 2: If we were to do scale first (non-standard)
    glm::mat4 M2 = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
    M2 = glm::translate(M2, glm::vec3(1.0f, 0.0f, 0.0f));

    // Apply to a test point
    glm::vec4 p1 = M1 * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 p2 = M2 * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    // Results should be different
    REQUIRE(std::abs(p1.x - p2.x) > EPS);
}

// ---------------------------------------------------------------------------
// Camera -- Integration tests
// ---------------------------------------------------------------------------
TEST_CASE("Camera can be moved and view matrix updates accordingly")
{
    Camera cam;
    glm::mat4 v1 = cam.viewMatrix();

    // Move camera
    cam.position = glm::vec3(10.0f, 5.0f, 10.0f);
    glm::mat4 v2 = cam.viewMatrix();

    // View matrices should be different
    REQUIRE(!matApprox(v1, v2));
}

TEST_CASE("Camera target change affects view matrix")
{
    Camera cam;
    glm::mat4 v1 = cam.viewMatrix();

    // Change what camera is looking at
    cam.target = glm::vec3(10.0f, 0.0f, 0.0f);
    glm::mat4 v2 = cam.viewMatrix();

    // View matrices should be different
    REQUIRE(!matApprox(v1, v2));
}

TEST_CASE("Camera with different FOV produces different projection matrices")
{
    Camera cam;

    cam.fov = 45.0f;
    glm::mat4 p1 = cam.projectionMatrix(1.0f);

    cam.fov = 90.0f;
    glm::mat4 p2 = cam.projectionMatrix(1.0f);

    // Projection matrices should be different
    REQUIRE(!matApprox(p1, p2));

    // Wider FOV should result in smaller scale values (zoomed out)
    REQUIRE(std::abs(p2[0][0]) < std::abs(p1[0][0]));
}

TEST_CASE("Camera near/far plane changes affect projection matrix")
{
    Camera cam;

    cam.nearPlane = 0.1f;
    cam.farPlane = 100.0f;
    glm::mat4 p1 = cam.projectionMatrix(1.0f);

    cam.nearPlane = 1.0f;
    cam.farPlane = 1000.0f;
    glm::mat4 p2 = cam.projectionMatrix(1.0f);

    // Projection matrices should be different
    REQUIRE(!matApprox(p1, p2));
}
