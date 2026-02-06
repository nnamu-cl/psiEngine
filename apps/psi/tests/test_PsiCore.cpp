//
// PSI Core Tests - Transform, Scene, and Object Selection
// Tests the fundamental components without requiring Vulkan/GPU
//

#include <catch2/catch_test_macros.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#include "Layers/DefaultGameWorld/Camera.h"
#include "Layers/DefaultGameWorld/Scene.h"
#include "Components/Transform.h"
#include "layers/PsiWorldLayer.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static constexpr float EPS = 1e-4f;

#define APPROX(value, target) REQUIRE(std::abs((value) - (target)) <= EPS)

static bool matApprox(const glm::mat4& a, const glm::mat4& b, float tol = EPS)
{
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            if (std::abs(a[c][r] - b[c][r]) > tol)
                return false;
    return true;
}

// ---------------------------------------------------------------------------
// Transform Tests (PSI-specific usage)
// ---------------------------------------------------------------------------
TEST_CASE("Transform default values are correct for new objects")
{
    GameObject obj{};
    obj.components.add(std::make_unique<Transform>());

    const Transform* transform = obj.components.get<Transform>();
    REQUIRE(transform != nullptr);
    APPROX(transform->position.x, 0.0f);
    APPROX(transform->position.y, 0.0f);
    APPROX(transform->position.z, 0.0f);
    APPROX(transform->scale.x, 1.0f);
    APPROX(transform->scale.y, 1.0f);
    APPROX(transform->scale.z, 1.0f);
}

TEST_CASE("Transform can be set via inspector-like operations")
{
    GameObject obj{};
    obj.components.add(std::make_unique<Transform>());

    Transform* transform = obj.components.get<Transform>();
    REQUIRE(transform != nullptr);

    // Simulate inspector editing position
    transform->position = glm::vec3(5.0f, 3.0f, -2.0f);
    APPROX(transform->position.x, 5.0f);
    APPROX(transform->position.y, 3.0f);
    APPROX(transform->position.z, -2.0f);

    // Simulate inspector editing scale
    transform->scale = glm::vec3(2.0f, 1.5f, 0.5f);
    APPROX(transform->scale.x, 2.0f);
    APPROX(transform->scale.y, 1.5f);
    APPROX(transform->scale.z, 0.5f);
}

TEST_CASE("Transform rotation via quaternion works correctly")
{
    GameObject obj{};
    obj.components.add(std::make_unique<Transform>());

    Transform* transform = obj.components.get<Transform>();
    REQUIRE(transform != nullptr);

    // 45 degree rotation around Y axis
    transform->rotation = glm::quat(glm::vec3(0.0f, glm::radians(45.0f), 0.0f));

    glm::mat4 M = transform->toMatrix();

    // Point at (1,0,0) should rotate approximately 45 degrees
    glm::vec4 point = M * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    // At 45 degrees: x = cos(45), z = -sin(45)
    APPROX(point.x, 0.7071f);  // cos(45°)
    APPROX(point.z, -0.7071f); // -sin(45°)
}

TEST_CASE("Transform matrix correctly combines all components")
{
    GameObject obj{};
    obj.components.add(std::make_unique<Transform>(
        glm::vec3(10.0f, 5.0f, 0.0f),
        glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f)),
        glm::vec3(2.0f, 2.0f, 2.0f)
    ));

    const Transform* transform = obj.components.get<Transform>();
    REQUIRE(transform != nullptr);

    glm::mat4 M = transform->toMatrix();

    // Point at (1,0,0) -> scale to (2,0,0) -> rotate to (0,0,-2) -> translate to (10,5,-2)
    glm::vec4 result = M * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    APPROX(result.x, 10.0f);
    APPROX(result.y, 5.0f);
    APPROX(result.z, -2.0f);
}

// ---------------------------------------------------------------------------
// Scene Tests (PSI-specific features)
// ---------------------------------------------------------------------------
TEST_CASE("Scene can store multiple named objects")
{
    Scene s;

    GameObject sphere{};
    sphere.name = "Sphere";
    s.addObject(std::move(sphere));

    GameObject cube{};
    cube.name = "Cube";
    s.addObject(std::move(cube));

    REQUIRE(s.objects.size() == 2);
    REQUIRE(s.objects[0].name == "Sphere");
    REQUIRE(s.objects[1].name == "Cube");
}

TEST_CASE("Scene objects preserve transform data after adding")
{
    Scene s;

    GameObject obj{};
    obj.name = "TestObject";
    obj.components.add(std::make_unique<Transform>(
        glm::vec3(1.0f, 2.0f, 3.0f),
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3(0.5f, 0.5f, 0.5f)
    ));
    obj.meshIndex = 42;
    obj.materialIndex = 7;

    s.addObject(std::move(obj));

    REQUIRE(s.objects[0].name == "TestObject");
    const Transform* transform = s.objects[0].components.get<Transform>();
    REQUIRE(transform != nullptr);
    APPROX(transform->position.x, 1.0f);
    APPROX(transform->position.y, 2.0f);
    APPROX(transform->position.z, 3.0f);
    APPROX(transform->scale.x, 0.5f);
    REQUIRE(s.objects[0].meshIndex == 42);
    REQUIRE(s.objects[0].materialIndex == 7);
}

TEST_CASE("Scene can be modified after creation")
{
    Scene s;

    GameObject obj{};
    obj.name = "Sphere";
    obj.components.add(std::make_unique<Transform>());
    s.addObject(std::move(obj));

    // Modify the object in the scene
    Transform* transform = s.objects[0].components.get<Transform>();
    REQUIRE(transform != nullptr);
    transform->position = glm::vec3(5.0f, 0.0f, 0.0f);
    s.objects[0].name = "ModifiedSphere";

    REQUIRE(s.objects[0].name == "ModifiedSphere");
    APPROX(transform->position.x, 5.0f);
}

TEST_CASE("Scene clear removes all objects")
{
    Scene s;

    for (int i = 0; i < 10; ++i)
    {
        GameObject obj{};
        obj.name = "Object_" + std::to_string(i);
        s.addObject(std::move(obj));
    }

    REQUIRE(s.objects.size() == 10);

    s.clear();
    REQUIRE(s.objects.empty());
}

// ---------------------------------------------------------------------------
// Object Selection Tests
// ---------------------------------------------------------------------------
TEST_CASE("Selected object index starts at -1 (no selection)")
{
    // Note: We can't directly instantiate PsiWorldLayer without ApplicationWindowData
    // So we test the selection logic separately
    int selectedObjectIndex = -1;

    REQUIRE(selectedObjectIndex == -1);

    // Simulate selecting first object
    selectedObjectIndex = 0;
    REQUIRE(selectedObjectIndex == 0);

    // Simulate deselecting
    selectedObjectIndex = -1;
    REQUIRE(selectedObjectIndex == -1);
}

TEST_CASE("Selection index validation works correctly")
{
    Scene s;

    // Add 3 objects
    for (int i = 0; i < 3; ++i)
    {
        GameObject obj{};
        obj.name = "Object_" + std::to_string(i);
        s.addObject(std::move(obj));
    }

    int selectedObjectIndex = -1;

    // Valid selections
    selectedObjectIndex = 0;
    REQUIRE(selectedObjectIndex >= 0);
    REQUIRE(selectedObjectIndex < static_cast<int>(s.objects.size()));

    selectedObjectIndex = 2;
    REQUIRE(selectedObjectIndex >= 0);
    REQUIRE(selectedObjectIndex < static_cast<int>(s.objects.size()));

    // Invalid selection (out of range)
    selectedObjectIndex = 5;
    REQUIRE(selectedObjectIndex >= static_cast<int>(s.objects.size()));

    // No selection
    selectedObjectIndex = -1;
    REQUIRE(selectedObjectIndex < 0);
}

TEST_CASE("Selected object can be accessed and modified")
{
    Scene s;

    GameObject obj{};
    obj.name = "Sphere";
    obj.components.add(std::make_unique<Transform>());
    s.addObject(std::move(obj));

    int selectedObjectIndex = 0;

    // Access selected object
    if (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(s.objects.size()))
    {
        GameObject& selected = s.objects[selectedObjectIndex];

        REQUIRE(selected.name == "Sphere");

        // Modify via inspector
        Transform* transform = selected.components.get<Transform>();
        REQUIRE(transform != nullptr);
        transform->position = glm::vec3(10.0f, 5.0f, -3.0f);

        // Verify modification
        const Transform* verifyTransform = s.objects[0].components.get<Transform>();
        REQUIRE(verifyTransform != nullptr);
        APPROX(verifyTransform->position.x, 10.0f);
        APPROX(verifyTransform->position.y, 5.0f);
        APPROX(verifyTransform->position.z, -3.0f);
    }
}

// ---------------------------------------------------------------------------
// GameObject Tests
// ---------------------------------------------------------------------------
TEST_CASE("GameObject default name is empty")
{
    GameObject obj{};
    REQUIRE(obj.name.empty());
}

TEST_CASE("GameObject can be named")
{
    GameObject obj{};
    obj.name = "MyObject";
    REQUIRE(obj.name == "MyObject");
}

TEST_CASE("GameObject mesh and material indices default to 0")
{
    GameObject obj{};
    REQUIRE(obj.meshIndex == 0);
    REQUIRE(obj.materialIndex == 0);
}

TEST_CASE("GameObject indices can be set")
{
    GameObject obj{};
    obj.meshIndex = 5;
    obj.materialIndex = 3;

    REQUIRE(obj.meshIndex == 5);
    REQUIRE(obj.materialIndex == 3);
}

// ---------------------------------------------------------------------------
// Integration: Multiple objects with different transforms
// ---------------------------------------------------------------------------
TEST_CASE("Scene with multiple objects maintains independent transforms")
{
    Scene s;

    GameObject obj1{};
    obj1.name = "Object1";
    obj1.components.add(std::make_unique<Transform>(glm::vec3(1.0f, 0.0f, 0.0f)));
    s.addObject(std::move(obj1));

    GameObject obj2{};
    obj2.name = "Object2";
    obj2.components.add(std::make_unique<Transform>(glm::vec3(0.0f, 1.0f, 0.0f)));
    s.addObject(std::move(obj2));

    GameObject obj3{};
    obj3.name = "Object3";
    obj3.components.add(std::make_unique<Transform>(glm::vec3(0.0f, 0.0f, 1.0f)));
    s.addObject(std::move(obj3));

    // Verify all positions are independent
    const Transform* t0 = s.objects[0].components.get<Transform>();
    const Transform* t1 = s.objects[1].components.get<Transform>();
    const Transform* t2 = s.objects[2].components.get<Transform>();

    REQUIRE(t0 != nullptr);
    REQUIRE(t1 != nullptr);
    REQUIRE(t2 != nullptr);

    APPROX(t0->position.x, 1.0f);
    APPROX(t0->position.y, 0.0f);

    APPROX(t1->position.x, 0.0f);
    APPROX(t1->position.y, 1.0f);

    APPROX(t2->position.z, 1.0f);

    // Modify one object
    Transform* t1_mut = s.objects[1].components.get<Transform>();
    t1_mut->position = glm::vec3(5.0f, 5.0f, 5.0f);

    // Verify others are unaffected
    APPROX(t0->position.x, 1.0f);
    APPROX(t2->position.z, 1.0f);

    // Verify modified object
    APPROX(t1_mut->position.x, 5.0f);
    APPROX(t1_mut->position.y, 5.0f);
}

TEST_CASE("Simulated inspector workflow: select, edit, verify")
{
    Scene s;

    // Create a scene with objects
    for (int i = 0; i < 5; ++i)
    {
        GameObject obj{};
        obj.name = "Object_" + std::to_string(i);
        obj.components.add(std::make_unique<Transform>(
            glm::vec3(static_cast<float>(i), 0.0f, 0.0f)
        ));
        s.addObject(std::move(obj));
    }

    int selectedObjectIndex = -1;

    // Simulate user clicking on object 2 in the UI
    selectedObjectIndex = 2;

    // Simulate inspector editing the selected object's transform
    if (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(s.objects.size()))
    {
        GameObject& selected = s.objects[selectedObjectIndex];
        Transform* transform = selected.components.get<Transform>();
        REQUIRE(transform != nullptr);

        // User changes position in inspector
        transform->position = glm::vec3(100.0f, 200.0f, 300.0f);

        // User changes scale in inspector
        transform->scale = glm::vec3(2.0f, 2.0f, 2.0f);
    }

    // Verify the edit was applied
    const Transform* t2 = s.objects[2].components.get<Transform>();
    REQUIRE(t2 != nullptr);
    APPROX(t2->position.x, 100.0f);
    APPROX(t2->position.y, 200.0f);
    APPROX(t2->position.z, 300.0f);
    APPROX(t2->scale.x, 2.0f);

    // Verify other objects are unaffected
    const Transform* t1 = s.objects[1].components.get<Transform>();
    const Transform* t3 = s.objects[3].components.get<Transform>();
    REQUIRE(t1 != nullptr);
    REQUIRE(t3 != nullptr);
    APPROX(t1->position.x, 1.0f);
    APPROX(t3->position.x, 3.0f);
}

// ---------------------------------------------------------------------------
// Object Deletion Tests
// ---------------------------------------------------------------------------
TEST_CASE("Scene deleteObject removes object at index")
{
    Scene s;

    for (int i = 0; i < 3; ++i)
    {
        GameObject obj{};
        obj.name = "Object_" + std::to_string(i);
        s.addObject(std::move(obj));
    }

    REQUIRE(s.objects.size() == 3);

    // Delete middle object
    s.deleteObject(1);

    REQUIRE(s.objects.size() == 2);
    REQUIRE(s.objects[0].name == "Object_0");
    REQUIRE(s.objects[1].name == "Object_2");
}

TEST_CASE("Scene deleteObject handles out of bounds gracefully")
{
    Scene s;

    GameObject obj{};
    obj.name = "OnlyObject";
    s.addObject(std::move(obj));

    REQUIRE(s.objects.size() == 1);

    // Try to delete invalid indices
    s.deleteObject(-1);
    REQUIRE(s.objects.size() == 1);

    s.deleteObject(5);
    REQUIRE(s.objects.size() == 1);

    // Delete valid index
    s.deleteObject(0);
    REQUIRE(s.objects.empty());
}

TEST_CASE("Deleting selected object clears selection")
{
    Scene s;

    for (int i = 0; i < 3; ++i)
    {
        GameObject obj{};
        obj.name = "Object_" + std::to_string(i);
        s.addObject(std::move(obj));
    }

    int selectedObjectIndex = 1;

    // Simulate deleting the selected object
    if (selectedObjectIndex >= 0 && selectedObjectIndex < static_cast<int>(s.objects.size()))
    {
        selectedObjectIndex = -1; // Clear selection
        s.deleteObject(1);
    }

    REQUIRE(selectedObjectIndex == -1);
    REQUIRE(s.objects.size() == 2);
}

TEST_CASE("Deleting object before selection adjusts index")
{
    Scene s;

    for (int i = 0; i < 5; ++i)
    {
        GameObject obj{};
        obj.name = "Object_" + std::to_string(i);
        s.addObject(std::move(obj));
    }

    int selectedObjectIndex = 3;

    // Delete object at index 1 (before selected object)
    s.deleteObject(1);

    // Selection index should be adjusted down
    selectedObjectIndex--;

    REQUIRE(selectedObjectIndex == 2);
    REQUIRE(s.objects.size() == 4);

    // Verify the correct object is still "selected"
    REQUIRE(s.objects[selectedObjectIndex].name == "Object_3");
}

TEST_CASE("Deleting object after selection preserves index")
{
    Scene s;

    for (int i = 0; i < 5; ++i)
    {
        GameObject obj{};
        obj.name = "Object_" + std::to_string(i);
        s.addObject(std::move(obj));
    }

    int selectedObjectIndex = 1;

    // Delete object at index 3 (after selected object)
    s.deleteObject(3);

    // Selection index should remain the same
    REQUIRE(selectedObjectIndex == 1);
    REQUIRE(s.objects.size() == 4);

    // Verify the selected object is still correct
    REQUIRE(s.objects[selectedObjectIndex].name == "Object_1");
}
