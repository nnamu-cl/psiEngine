//
// Node Graph Tests - GameObjectNodeDrawer and Component Node UI
// Tests the node graph system without requiring Vulkan/GPU or ImGui context
//

#include <catch2/catch_test_macros.hpp>

#include "drawers/GameObjectNodeDrawer.h"
#include "Layers/DefaultGameWorld/Scene.h"
#include "Components/Transform.h"
#include "Components/MeshRenderer.h"

// ---------------------------------------------------------------------------
// High Priority Tests
// ---------------------------------------------------------------------------

// ===========================================================================
// 1. Construction & Basic State
// ===========================================================================

TEST_CASE("GameObjectNodeDrawer constructs with valid scene pointer")
{
    Scene scene;
    GameObjectNodeDrawer drawer(&scene);

    // Should construct without throwing
    REQUIRE(drawer.GetNodeCount() == 0);
}

TEST_CASE("GameObjectNodeDrawer constructs with null scene pointer")
{
    GameObjectNodeDrawer drawer(nullptr);

    // Should construct without throwing
    REQUIRE(drawer.GetNodeCount() == 0);
}

// ===========================================================================
// 2. GetNodeCount Tests
// ===========================================================================

TEST_CASE("GetNodeCount returns 0 for empty scene")
{
    Scene emptyScene;
    GameObjectNodeDrawer drawer(&emptyScene);

    REQUIRE(drawer.GetNodeCount() == 0);
}

TEST_CASE("GetNodeCount returns correct count for scene with one object")
{
    Scene scene;

    GameObject obj;
    obj.name = "TestObject";
    scene.addObject(std::move(obj));

    GameObjectNodeDrawer drawer(&scene);

    REQUIRE(drawer.GetNodeCount() == 1);
}

TEST_CASE("GetNodeCount returns correct count for scene with multiple objects")
{
    Scene scene;

    for (int i = 0; i < 5; ++i)
    {
        GameObject obj;
        obj.name = "Object_" + std::to_string(i);
        scene.addObject(std::move(obj));
    }

    GameObjectNodeDrawer drawer(&scene);

    REQUIRE(drawer.GetNodeCount() == 5);
}

TEST_CASE("GetNodeCount updates when scene objects are added")
{
    Scene scene;
    GameObjectNodeDrawer drawer(&scene);

    REQUIRE(drawer.GetNodeCount() == 0);

    GameObject obj1;
    obj1.name = "Object1";
    scene.addObject(std::move(obj1));

    REQUIRE(drawer.GetNodeCount() == 1);

    GameObject obj2;
    obj2.name = "Object2";
    scene.addObject(std::move(obj2));

    REQUIRE(drawer.GetNodeCount() == 2);
}

TEST_CASE("GetNodeCount updates when scene objects are removed")
{
    Scene scene;

    for (int i = 0; i < 3; ++i)
    {
        GameObject obj;
        obj.name = "Object_" + std::to_string(i);
        scene.addObject(std::move(obj));
    }

    GameObjectNodeDrawer drawer(&scene);
    REQUIRE(drawer.GetNodeCount() == 3);

    scene.deleteObject(1);
    REQUIRE(drawer.GetNodeCount() == 2);

    scene.clear();
    REQUIRE(drawer.GetNodeCount() == 0);
}

// ===========================================================================
// 3. Unique ID Generation (Tested Indirectly)
// ===========================================================================

TEST_CASE("GeneratePinId produces different IDs for different node/pin combinations")
{
    // Note: GeneratePinId is private, so we test it indirectly by ensuring
    // that the drawer can handle multiple nodes without ID conflicts
    Scene scene;

    for (int i = 0; i < 10; ++i)
    {
        GameObject obj;
        obj.name = "Object_" + std::to_string(i);
        scene.addObject(std::move(obj));
    }

    GameObjectNodeDrawer drawer(&scene);

    // If pin IDs are not unique, DrawNode would have issues
    // This test ensures the drawer can handle multiple objects
    REQUIRE(drawer.GetNodeCount() == 10);

    // Call DrawNode for each object - should not crash
    for (size_t i = 0; i < drawer.GetNodeCount(); ++i)
    {
        REQUIRE_NOTHROW(drawer.DrawNode(i));
    }
}

// ===========================================================================
// 4. Null Safety Tests
// ===========================================================================

TEST_CASE("GameObjectNodeDrawer handles null scene pointer in GetNodeCount")
{
    GameObjectNodeDrawer drawer(nullptr);

    REQUIRE(drawer.GetNodeCount() == 0);
}

TEST_CASE("GameObjectNodeDrawer handles null scene pointer in DrawNode")
{
    GameObjectNodeDrawer drawer(nullptr);

    // Should not crash when trying to draw with null scene
    REQUIRE_NOTHROW(drawer.DrawNode(0));
}

TEST_CASE("GameObjectNodeDrawer handles null scene pointer in DrawLinks")
{
    GameObjectNodeDrawer drawer(nullptr);

    // Should not crash
    REQUIRE_NOTHROW(drawer.DrawLinks());
}

TEST_CASE("GameObjectNodeDrawer handles null scene pointer in HandleInteractions")
{
    GameObjectNodeDrawer drawer(nullptr);

    // Should not crash
    REQUIRE_NOTHROW(drawer.HandleInteractions());
}

// ===========================================================================
// 5. Component Access Tests
// ===========================================================================

TEST_CASE("DrawNode handles GameObject with Transform component")
{
    Scene scene;

    GameObject obj;
    obj.name = "ObjectWithTransform";
    obj.components.add(std::make_unique<Transform>(
        glm::vec3(1.0f, 2.0f, 3.0f)
    ));
    scene.addObject(std::move(obj));

    GameObjectNodeDrawer drawer(&scene);

    // Should not crash when drawing object with Transform
    REQUIRE_NOTHROW(drawer.DrawNode(0));
}

TEST_CASE("DrawNode handles GameObject with MeshRenderer component")
{
    Scene scene;

    GameObject obj;
    obj.name = "ObjectWithMeshRenderer";
    obj.components.add(std::make_unique<MeshRenderer>(
        ColorMode::ObjectColor,
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
    ));
    scene.addObject(std::move(obj));

    GameObjectNodeDrawer drawer(&scene);

    // Should not crash when drawing object with MeshRenderer
    REQUIRE_NOTHROW(drawer.DrawNode(0));
}

TEST_CASE("DrawNode handles GameObject with no components")
{
    Scene scene;

    GameObject obj;
    obj.name = "EmptyObject";
    scene.addObject(std::move(obj));

    GameObjectNodeDrawer drawer(&scene);

    // Should not crash when drawing object without components
    REQUIRE_NOTHROW(drawer.DrawNode(0));
}

TEST_CASE("DrawNode handles GameObject with multiple components")
{
    Scene scene;

    GameObject obj;
    obj.name = "FullObject";
    obj.components.add(std::make_unique<Transform>(
        glm::vec3(5.0f, 10.0f, 15.0f),
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 2.0f, 2.0f)
    ));
    obj.components.add(std::make_unique<MeshRenderer>(
        ColorMode::VertexColor
    ));
    scene.addObject(std::move(obj));

    GameObjectNodeDrawer drawer(&scene);

    // Should not crash when drawing object with multiple components
    REQUIRE_NOTHROW(drawer.DrawNode(0));
}

TEST_CASE("DrawNode accesses correct GameObject by index")
{
    Scene scene;

    GameObject obj1;
    obj1.name = "FirstObject";
    obj1.meshIndex = 10;
    scene.addObject(std::move(obj1));

    GameObject obj2;
    obj2.name = "SecondObject";
    obj2.meshIndex = 20;
    scene.addObject(std::move(obj2));

    GameObject obj3;
    obj3.name = "ThirdObject";
    obj3.meshIndex = 30;
    scene.addObject(std::move(obj3));

    GameObjectNodeDrawer drawer(&scene);

    // Verify we can access all objects
    REQUIRE(drawer.GetNodeCount() == 3);

    // Verify meshIndex values are preserved (indirect test that correct objects are accessed)
    REQUIRE(scene.objects[0].meshIndex == 10);
    REQUIRE(scene.objects[1].meshIndex == 20);
    REQUIRE(scene.objects[2].meshIndex == 30);

    // All DrawNode calls should work
    REQUIRE_NOTHROW(drawer.DrawNode(0));
    REQUIRE_NOTHROW(drawer.DrawNode(1));
    REQUIRE_NOTHROW(drawer.DrawNode(2));
}

// ===========================================================================
// Node Access & Bounds Checking
// ===========================================================================

TEST_CASE("DrawNode with valid node ID does not crash")
{
    Scene scene;

    for (int i = 0; i < 5; ++i)
    {
        GameObject obj;
        obj.name = "Object_" + std::to_string(i);
        scene.addObject(std::move(obj));
    }

    GameObjectNodeDrawer drawer(&scene);

    // All valid indices should work
    for (size_t i = 0; i < drawer.GetNodeCount(); ++i)
    {
        REQUIRE_NOTHROW(drawer.DrawNode(i));
    }
}

TEST_CASE("DrawNode with invalid node ID does not crash")
{
    Scene scene;

    GameObject obj;
    obj.name = "SingleObject";
    scene.addObject(std::move(obj));

    GameObjectNodeDrawer drawer(&scene);

    // Out of bounds access should not crash
    REQUIRE_NOTHROW(drawer.DrawNode(999));
    REQUIRE_NOTHROW(drawer.DrawNode(100000));
}

TEST_CASE("DrawNode with node ID at boundary does not crash")
{
    Scene scene;

    for (int i = 0; i < 3; ++i)
    {
        GameObject obj;
        obj.name = "Object_" + std::to_string(i);
        scene.addObject(std::move(obj));
    }

    GameObjectNodeDrawer drawer(&scene);

    // Test boundary: last valid index
    REQUIRE_NOTHROW(drawer.DrawNode(2));

    // Test boundary: first invalid index
    REQUIRE_NOTHROW(drawer.DrawNode(3));
}

// ===========================================================================
// Component DrawNodeUI Tests
// ===========================================================================

TEST_CASE("Transform DrawNodeUI can be called with default values")
{
    Transform transform;

    // Should not crash with default values
    REQUIRE_NOTHROW(transform.DrawNodeUI());
}

TEST_CASE("Transform DrawNodeUI can be called with custom position")
{
    Transform transform(glm::vec3(10.0f, 20.0f, 30.0f));

    // Should not crash with custom position
    REQUIRE_NOTHROW(transform.DrawNodeUI());
}

TEST_CASE("Transform DrawNodeUI can be called with custom rotation")
{
    Transform transform(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::quat(glm::vec3(glm::radians(45.0f), glm::radians(90.0f), glm::radians(135.0f)))
    );

    // Should not crash with custom rotation
    REQUIRE_NOTHROW(transform.DrawNodeUI());
}

TEST_CASE("Transform DrawNodeUI can be called with custom scale")
{
    Transform transform(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 3.0f, 4.0f)
    );

    // Should not crash with custom scale
    REQUIRE_NOTHROW(transform.DrawNodeUI());
}

TEST_CASE("Transform DrawNodeUI can be called with extreme values")
{
    Transform transform(
        glm::vec3(1000000.0f, -1000000.0f, 0.0f),
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        glm::vec3(0.001f, 1000.0f, 1.0f)
    );

    // Should not crash with extreme values
    REQUIRE_NOTHROW(transform.DrawNodeUI());
}

TEST_CASE("MeshRenderer DrawNodeUI can be called with default values")
{
    MeshRenderer renderer;

    // Should not crash with default values
    REQUIRE_NOTHROW(renderer.DrawNodeUI());
}

TEST_CASE("MeshRenderer DrawNodeUI can be called with VertexColor mode")
{
    MeshRenderer renderer(ColorMode::VertexColor);

    // Should not crash in VertexColor mode
    REQUIRE_NOTHROW(renderer.DrawNodeUI());
}

TEST_CASE("MeshRenderer DrawNodeUI can be called with ObjectColor mode")
{
    MeshRenderer renderer(
        ColorMode::ObjectColor,
        glm::vec4(0.5f, 0.3f, 0.8f, 1.0f)
    );

    // Should not crash in ObjectColor mode
    REQUIRE_NOTHROW(renderer.DrawNodeUI());
}

// ===========================================================================
// Empty Scene Handling
// ===========================================================================

TEST_CASE("DrawLinks on empty scene does not crash")
{
    Scene emptyScene;
    GameObjectNodeDrawer drawer(&emptyScene);

    REQUIRE_NOTHROW(drawer.DrawLinks());
}

TEST_CASE("HandleInteractions on empty scene does not crash")
{
    Scene emptyScene;
    GameObjectNodeDrawer drawer(&emptyScene);

    REQUIRE_NOTHROW(drawer.HandleInteractions());
}

// ===========================================================================
// Scene Modification Tests
// ===========================================================================

TEST_CASE("Node drawer reflects scene additions")
{
    Scene scene;
    GameObjectNodeDrawer drawer(&scene);

    REQUIRE(drawer.GetNodeCount() == 0);

    GameObject obj1;
    obj1.name = "Object1";
    scene.addObject(std::move(obj1));

    REQUIRE(drawer.GetNodeCount() == 1);

    GameObject obj2;
    obj2.name = "Object2";
    scene.addObject(std::move(obj2));

    REQUIRE(drawer.GetNodeCount() == 2);
}

TEST_CASE("Node drawer reflects scene deletions")
{
    Scene scene;

    for (int i = 0; i < 5; ++i)
    {
        GameObject obj;
        obj.name = "Object_" + std::to_string(i);
        scene.addObject(std::move(obj));
    }

    GameObjectNodeDrawer drawer(&scene);
    REQUIRE(drawer.GetNodeCount() == 5);

    scene.deleteObject(0);
    REQUIRE(drawer.GetNodeCount() == 4);

    scene.deleteObject(0);
    REQUIRE(drawer.GetNodeCount() == 3);
}

TEST_CASE("Node drawer handles scene clear operation")
{
    Scene scene;

    for (int i = 0; i < 10; ++i)
    {
        GameObject obj;
        obj.name = "Object_" + std::to_string(i);
        scene.addObject(std::move(obj));
    }

    GameObjectNodeDrawer drawer(&scene);
    REQUIRE(drawer.GetNodeCount() == 10);

    scene.clear();
    REQUIRE(drawer.GetNodeCount() == 0);
}

// ===========================================================================
// Large Dataset Tests
// ===========================================================================

TEST_CASE("Node drawer handles scene with many objects")
{
    Scene scene;

    // Add 100 objects
    for (int i = 0; i < 100; ++i)
    {
        GameObject obj;
        obj.name = "Object_" + std::to_string(i);
        obj.meshIndex = static_cast<uint32_t>(i);
        scene.addObject(std::move(obj));
    }

    GameObjectNodeDrawer drawer(&scene);

    REQUIRE(drawer.GetNodeCount() == 100);

    // Should be able to draw all nodes without crashing
    for (size_t i = 0; i < drawer.GetNodeCount(); ++i)
    {
        REQUIRE_NOTHROW(drawer.DrawNode(i));
    }
}

TEST_CASE("Node drawer handles objects with all component combinations")
{
    Scene scene;

    // Object with no components
    GameObject obj1;
    obj1.name = "NoComponents";
    scene.addObject(std::move(obj1));

    // Object with only Transform
    GameObject obj2;
    obj2.name = "OnlyTransform";
    obj2.components.add(std::make_unique<Transform>());
    scene.addObject(std::move(obj2));

    // Object with only MeshRenderer
    GameObject obj3;
    obj3.name = "OnlyMeshRenderer";
    obj3.components.add(std::make_unique<MeshRenderer>());
    scene.addObject(std::move(obj3));

    // Object with both Transform and MeshRenderer
    GameObject obj4;
    obj4.name = "BothComponents";
    obj4.components.add(std::make_unique<Transform>());
    obj4.components.add(std::make_unique<MeshRenderer>());
    scene.addObject(std::move(obj4));

    GameObjectNodeDrawer drawer(&scene);

    REQUIRE(drawer.GetNodeCount() == 4);

    // All objects should render without crashing
    for (size_t i = 0; i < drawer.GetNodeCount(); ++i)
    {
        REQUIRE_NOTHROW(drawer.DrawNode(i));
    }
}
