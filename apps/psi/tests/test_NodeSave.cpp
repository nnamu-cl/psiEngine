//
// Node Save/Load Tests
// Tests the NodeGraph::Save() / Load() round-trip without requiring Vulkan/GPU.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nodes/NodeSystem.h"
#include "nodes/ValueNodes.h"
#include "nodes/MathNodes.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;
using Catch::Matchers::WithinAbs;

// ---------------------------------------------------------------------------
// Helper: create (and later clean up) an isolated temp directory per test.
// ---------------------------------------------------------------------------
static fs::path makeTmpDir(const std::string& suffix)
{
    fs::path dir = fs::temp_directory_path() / ("psi_nodesave_" + suffix);
    fs::create_directories(dir);
    return dir;
}

// ---------------------------------------------------------------------------
// TEST 1: Saving an empty graph produces a file; loading it yields 0 nodes.
// ---------------------------------------------------------------------------
TEST_CASE("Empty graph saves to disk and loads back as empty", "[NodeSave]")
{
    auto dir = makeTmpDir("empty");

    NodeGraph saveGraph;
    saveGraph.Save(dir.string());

    REQUIRE(fs::exists(dir / "graph.json"));

    NodeGraph loadGraph;
    loadGraph.Load((dir / "graph.json").string());

    REQUIRE(loadGraph.getNodes().empty());

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 2: A single node's type name survives the round-trip.
// ---------------------------------------------------------------------------
TEST_CASE("Single node type name survives round-trip", "[NodeSave]")
{
    auto dir = makeTmpDir("typename");

    NodeGraph saveGraph;
    saveGraph.createNode<FloatConstantNode>(1.0f);
    saveGraph.Save(dir.string());

    NodeGraph loadGraph;
    loadGraph.Load((dir / "graph.json").string());

    REQUIRE(loadGraph.getNodes().size() == 1);
    REQUIRE(std::string(loadGraph.getNodes()[0]->getTypeName()) == "Float");

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 3: FloatConstantNode's value is preserved via SaveProperties/LoadProperties.
// ---------------------------------------------------------------------------
TEST_CASE("FloatConstantNode value is preserved after round-trip", "[NodeSave]")
{
    auto dir = makeTmpDir("floatval");

    NodeGraph saveGraph;
    saveGraph.createNode<FloatConstantNode>(3.14f);
    saveGraph.Save(dir.string());

    NodeGraph loadGraph;
    loadGraph.Load((dir / "graph.json").string());

    REQUIRE(loadGraph.getNodes().size() == 1);
    auto* loaded = dynamic_cast<FloatConstantNode*>(loadGraph.getNodes()[0].get());
    REQUIRE(loaded != nullptr);
    REQUIRE_THAT(loaded->getValue(), WithinAbs(3.14f, 0.001f));

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 4: Multiple nodes with different types all survive with correct count.
// ---------------------------------------------------------------------------
TEST_CASE("Multiple nodes survive round-trip with correct count and types", "[NodeSave]")
{
    auto dir = makeTmpDir("multi");

    NodeGraph saveGraph;
    saveGraph.createNode<FloatConstantNode>(1.0f);
    saveGraph.createNode<FloatConstantNode>(2.0f);
    saveGraph.createNode<AddNode>();
    saveGraph.Save(dir.string());

    NodeGraph loadGraph;
    loadGraph.Load((dir / "graph.json").string());

    REQUIRE(loadGraph.getNodes().size() == 3);

    int floatCount = 0, addCount = 0;
    for (const auto& n : loadGraph.getNodes())
    {
        if (std::string(n->getTypeName()) == "Float") ++floatCount;
        if (std::string(n->getTypeName()) == "Add")   ++addCount;
    }
    REQUIRE(floatCount == 2);
    REQUIRE(addCount == 1);

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 5: A custom display name set via setName() is preserved.
// ---------------------------------------------------------------------------
TEST_CASE("Custom node display name survives round-trip", "[NodeSave]")
{
    auto dir = makeTmpDir("customname");

    NodeGraph saveGraph;
    auto* node = saveGraph.createNode<FloatConstantNode>(0.0f);
    node->setName("GravityScalar");
    saveGraph.Save(dir.string());

    NodeGraph loadGraph;
    loadGraph.Load((dir / "graph.json").string());

    REQUIRE(loadGraph.getNodes().size() == 1);
    REQUIRE(loadGraph.getNodes()[0]->getName() == "GravityScalar");

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 6: The node's numeric ID is preserved exactly.
// ---------------------------------------------------------------------------
TEST_CASE("Node ID is preserved after round-trip", "[NodeSave]")
{
    auto dir = makeTmpDir("nodeid");

    NodeGraph saveGraph;
    auto* node = saveGraph.createNode<FloatConstantNode>(0.0f);
    uint64_t originalId = node->getId();
    saveGraph.Save(dir.string());

    NodeGraph loadGraph;
    loadGraph.Load((dir / "graph.json").string());

    REQUIRE(loadGraph.getNodes().size() == 1);
    REQUIRE(loadGraph.getNodes()[0]->getId() == originalId);

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 7: Connections between nodes are fully restored after round-trip.
//
// Graph: FloatA --[Value]--> AddNode <--[Value]-- FloatB
// After load both inputs of AddNode must be connected.
// ---------------------------------------------------------------------------
TEST_CASE("Connections between nodes are restored after round-trip", "[NodeSave]")
{
    auto dir = makeTmpDir("connections");

    NodeGraph saveGraph;
    auto* floatA = saveGraph.createNode<FloatConstantNode>(3.0f);
    auto* floatB = saveGraph.createNode<FloatConstantNode>(4.0f);
    auto* addNode = saveGraph.createNode<AddNode>();

    saveGraph.connect(floatA->getOutput("Value"), addNode->getInput("A"));
    saveGraph.connect(floatB->getOutput("Value"), addNode->getInput("B"));
    saveGraph.Save(dir.string());

    NodeGraph loadGraph;
    loadGraph.Load((dir / "graph.json").string());

    AddNode* loadedAdd = nullptr;
    for (const auto& n : loadGraph.getNodes())
    {
        if (auto* a = dynamic_cast<AddNode*>(n.get()))
        {
            loadedAdd = a;
            break;
        }
    }

    REQUIRE(loadedAdd != nullptr);
    REQUIRE(loadedAdd->getInput("A")->isConnected());
    REQUIRE(loadedAdd->getInput("B")->isConnected());

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 8: Loading into a graph that already has nodes clears the old ones.
// ---------------------------------------------------------------------------
TEST_CASE("Loading a graph clears any pre-existing nodes", "[NodeSave]")
{
    auto dir = makeTmpDir("clearold");

    // Save a one-node graph.
    NodeGraph saveGraph;
    saveGraph.createNode<FloatConstantNode>(7.0f);
    saveGraph.Save(dir.string());

    // Pre-populate the target graph with unrelated nodes.
    NodeGraph loadGraph;
    loadGraph.createNode<FloatConstantNode>(99.0f);
    loadGraph.createNode<AddNode>();
    loadGraph.createNode<AddNode>();
    REQUIRE(loadGraph.getNodes().size() == 3);

    // Load should wipe the old three nodes and rebuild from the file.
    loadGraph.Load((dir / "graph.json").string());

    REQUIRE(loadGraph.getNodes().size() == 1);
    auto* loaded = dynamic_cast<FloatConstantNode*>(loadGraph.getNodes()[0].get());
    REQUIRE(loaded != nullptr);
    REQUIRE_THAT(loaded->getValue(), WithinAbs(7.0f, 0.001f));

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 9: After loading, the internal ID counter advances past all loaded IDs
//         so that new nodes never collide with loaded ones.
// ---------------------------------------------------------------------------
TEST_CASE("New nodes get IDs that do not collide with loaded nodes", "[NodeSave]")
{
    auto dir = makeTmpDir("nextid");

    NodeGraph saveGraph;
    saveGraph.createNode<FloatConstantNode>(); // ID 1
    saveGraph.createNode<FloatConstantNode>(); // ID 2
    saveGraph.createNode<FloatConstantNode>(); // ID 3
    saveGraph.Save(dir.string());

    NodeGraph loadGraph;
    loadGraph.Load((dir / "graph.json").string());

    // Creating a new node should produce ID 4, not 1/2/3.
    auto* newNode = loadGraph.createNode<FloatConstantNode>();
    uint64_t newId = newNode->getId();

    for (const auto& n : loadGraph.getNodes())
    {
        if (n.get() != newNode)
            REQUIRE(n->getId() != newId);
    }
    REQUIRE(newId == 4);

    fs::remove_all(dir);
}

// ---------------------------------------------------------------------------
// TEST 10: Attempting to load a non-existent file is a safe no-op — the graph
//          is left completely unchanged.
// ---------------------------------------------------------------------------
TEST_CASE("Loading a non-existent file leaves the graph unchanged", "[NodeSave]")
{
    NodeGraph graph;
    graph.createNode<FloatConstantNode>(5.0f);

    graph.Load("C:/nonexistent/path/that/does/not/exist/graph.json");

    REQUIRE(graph.getNodes().size() == 1);
    auto* node = dynamic_cast<FloatConstantNode*>(graph.getNodes()[0].get());
    REQUIRE(node != nullptr);
    REQUIRE_THAT(node->getValue(), WithinAbs(5.0f, 0.001f));
}
