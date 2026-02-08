//
// Node Deletion Tests
// Tests the node deletion functionality in the node graph system
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "nodes/NodeSystem.h"
#include "nodes/ValueNodes.h"
#include "nodes/MathNodes.h"
#include "nodes/VectorNodes.h"

using Catch::Matchers::WithinAbs;

// ====================================================================================
// TEST 1: Delete Single Node With No Connections
// ====================================================================================
TEST_CASE("Delete single node with no connections", "[NodeGraph][NodeDeletion]") {
    NodeGraph graph;

    SECTION("Delete a standalone float constant node") {
        auto* floatNode = graph.createNode<FloatConstantNode>(5.0f);
        uint64_t nodeId = floatNode->getId();

        // Verify node exists
        REQUIRE(graph.getNodes().size() == 1);

        // Delete the node
        bool result = graph.deleteNode(nodeId);

        REQUIRE(result == true);
        REQUIRE(graph.getNodes().size() == 0);
    }

    SECTION("Delete multiple standalone nodes") {
        auto* node1 = graph.createNode<FloatConstantNode>(1.0f);
        auto* node2 = graph.createNode<FloatConstantNode>(2.0f);
        auto* node3 = graph.createNode<AddNode>();

        REQUIRE(graph.getNodes().size() == 3);

        // Delete first node
        bool result1 = graph.deleteNode(node1->getId());
        REQUIRE(result1 == true);
        REQUIRE(graph.getNodes().size() == 2);

        // Delete second node
        bool result2 = graph.deleteNode(node2->getId());
        REQUIRE(result2 == true);
        REQUIRE(graph.getNodes().size() == 1);

        // Delete third node
        bool result3 = graph.deleteNode(node3->getId());
        REQUIRE(result3 == true);
        REQUIRE(graph.getNodes().size() == 0);
    }

    SECTION("Attempt to delete non-existent node") {
        auto* floatNode = graph.createNode<FloatConstantNode>(5.0f);

        REQUIRE(graph.getNodes().size() == 1);

        // Try to delete a node ID that doesn't exist
        bool result = graph.deleteNode(99999);

        REQUIRE(result == false);
        REQUIRE(graph.getNodes().size() == 1); // Original node still exists
    }
}

// ====================================================================================
// TEST 2: Delete Node With Input Connections
// ====================================================================================
TEST_CASE("Delete node with input connections", "[NodeGraph][NodeDeletion]") {
    NodeGraph graph;

    SECTION("Delete node with one input connection") {
        auto* sourceNode = graph.createNode<FloatConstantNode>(10.0f);
        auto* targetNode = graph.createNode<AddNode>();

        // Connect source to target
        graph.connect(sourceNode->getOutput("Value"), targetNode->getInput("A"));

        // Verify connection exists
        REQUIRE(targetNode->getInput("A")->isConnected());
        REQUIRE(graph.getNodes().size() == 2);

        // Delete the target node (has input connection)
        bool result = graph.deleteNode(targetNode->getId());

        REQUIRE(result == true);
        REQUIRE(graph.getNodes().size() == 1);

        // Source node should still exist and be valid
        sourceNode->evaluate();
        auto* output = sourceNode->getOutput("Value");
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(10.0f, 0.001f));
    }

    SECTION("Delete node with multiple input connections") {
        auto* sourceNode1 = graph.createNode<FloatConstantNode>(5.0f);
        auto* sourceNode2 = graph.createNode<FloatConstantNode>(3.0f);
        auto* targetNode = graph.createNode<AddNode>();

        // Connect both sources to target
        graph.connect(sourceNode1->getOutput("Value"), targetNode->getInput("A"));
        graph.connect(sourceNode2->getOutput("Value"), targetNode->getInput("B"));

        REQUIRE(targetNode->getInput("A")->isConnected());
        REQUIRE(targetNode->getInput("B")->isConnected());
        REQUIRE(graph.getNodes().size() == 3);

        // Delete the target node
        bool result = graph.deleteNode(targetNode->getId());

        REQUIRE(result == true);
        REQUIRE(graph.getNodes().size() == 2);

        // Both source nodes should still be valid
        sourceNode1->evaluate();
        sourceNode2->evaluate();
        REQUIRE_THAT(std::get<float>(sourceNode1->getOutput("Value")->getValue()), WithinAbs(5.0f, 0.001f));
        REQUIRE_THAT(std::get<float>(sourceNode2->getOutput("Value")->getValue()), WithinAbs(3.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 3: Delete Node With Output Connections
// ====================================================================================
TEST_CASE("Delete node with output connections", "[NodeGraph][NodeDeletion]") {
    NodeGraph graph;

    SECTION("Delete node with one output connection") {
        auto* sourceNode = graph.createNode<FloatConstantNode>(7.0f);
        auto* targetNode = graph.createNode<SinNode>();

        // Connect source to target
        graph.connect(sourceNode->getOutput("Value"), targetNode->getInput("Value"));

        REQUIRE(targetNode->getInput("Value")->isConnected());
        REQUIRE(graph.getNodes().size() == 2);

        // Delete the source node (has output connection)
        bool result = graph.deleteNode(sourceNode->getId());

        REQUIRE(result == true);
        REQUIRE(graph.getNodes().size() == 1);

        // Target node should no longer be connected
        REQUIRE_FALSE(targetNode->getInput("Value")->isConnected());

        // Target node should use default value
        targetNode->evaluate();
        auto* output = targetNode->getOutput("Result");
        // Default input is 0.0f, sin(0) = 0
        REQUIRE_THAT(std::get<float>(output->getValue()), WithinAbs(0.0f, 0.001f));
    }

    SECTION("Delete node with multiple output connections (fan-out)") {
        auto* sourceNode = graph.createNode<FloatConstantNode>(2.0f);
        auto* targetNode1 = graph.createNode<AddNode>();
        auto* targetNode2 = graph.createNode<MultiplyNode>();
        auto* targetNode3 = graph.createNode<SinNode>();

        // Connect source to multiple targets
        graph.connect(sourceNode->getOutput("Value"), targetNode1->getInput("A"));
        graph.connect(sourceNode->getOutput("Value"), targetNode2->getInput("A"));
        graph.connect(sourceNode->getOutput("Value"), targetNode3->getInput("Value"));

        REQUIRE(targetNode1->getInput("A")->isConnected());
        REQUIRE(targetNode2->getInput("A")->isConnected());
        REQUIRE(targetNode3->getInput("Value")->isConnected());
        REQUIRE(graph.getNodes().size() == 4);

        // Delete the source node
        bool result = graph.deleteNode(sourceNode->getId());

        REQUIRE(result == true);
        REQUIRE(graph.getNodes().size() == 3);

        // All target nodes should be disconnected
        REQUIRE_FALSE(targetNode1->getInput("A")->isConnected());
        REQUIRE_FALSE(targetNode2->getInput("A")->isConnected());
        REQUIRE_FALSE(targetNode3->getInput("Value")->isConnected());

        // All target nodes should still be evaluable with defaults
        targetNode1->evaluate();
        targetNode2->evaluate();
        targetNode3->evaluate();

        // Default values: Add(0,0) = 0, Multiply(1,1) = 1, Sin(0) = 0
        REQUIRE_THAT(std::get<float>(targetNode1->getOutput("Result")->getValue()), WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(std::get<float>(targetNode2->getOutput("Result")->getValue()), WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(std::get<float>(targetNode3->getOutput("Result")->getValue()), WithinAbs(0.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 4: Delete Middle Node in Graph Chain
// ====================================================================================
TEST_CASE("Delete middle node in graph chain", "[NodeGraph][NodeDeletion]") {
    NodeGraph graph;

    SECTION("Delete middle node in simple chain: A -> B -> C") {
        auto* nodeA = graph.createNode<FloatConstantNode>(5.0f);
        auto* nodeB = graph.createNode<MultiplyNode>();
        auto* nodeC = graph.createNode<AddNode>();

        auto* constantTwo = graph.createNode<FloatConstantNode>(2.0f);
        auto* constantThree = graph.createNode<FloatConstantNode>(3.0f);

        // Build chain: nodeA(5) -> nodeB(*2) -> nodeC(+3)
        graph.connect(nodeA->getOutput("Value"), nodeB->getInput("A"));
        graph.connect(constantTwo->getOutput("Value"), nodeB->getInput("B"));
        graph.connect(nodeB->getOutput("Result"), nodeC->getInput("A"));
        graph.connect(constantThree->getOutput("Value"), nodeC->getInput("B"));

        REQUIRE(graph.getNodes().size() == 5);

        // Evaluate the chain: 5 * 2 = 10, then 10 + 3 = 13
        nodeA->evaluate();
        constantTwo->evaluate();
        nodeB->evaluate();
        constantThree->evaluate();
        nodeC->evaluate();
        REQUIRE_THAT(std::get<float>(nodeC->getOutput("Result")->getValue()), WithinAbs(13.0f, 0.001f));

        // Delete the middle node (nodeB)
        bool result = graph.deleteNode(nodeB->getId());

        REQUIRE(result == true);
        REQUIRE(graph.getNodes().size() == 4);

        // NodeA should still exist
        nodeA->evaluate();
        REQUIRE_THAT(std::get<float>(nodeA->getOutput("Value")->getValue()), WithinAbs(5.0f, 0.001f));

        // NodeC should be disconnected from nodeB
        REQUIRE_FALSE(nodeC->getInput("A")->isConnected());

        // NodeC should use default value for input A (0.0) + 3 = 3
        nodeC->markDirty();
        nodeC->evaluate();
        REQUIRE_THAT(std::get<float>(nodeC->getOutput("Result")->getValue()), WithinAbs(3.0f, 0.001f));
    }

    SECTION("Delete middle node in complex graph") {
        // Build a more complex graph:
        //    A(10) ──┐
        //            ├─> B(+) ──> C(*)
        //    D(5)  ──┘               │
        //                            v
        //                         E(Sin)

        auto* nodeA = graph.createNode<FloatConstantNode>(10.0f);
        auto* nodeD = graph.createNode<FloatConstantNode>(5.0f);
        auto* nodeB = graph.createNode<AddNode>();
        auto* nodeC = graph.createNode<MultiplyNode>();
        auto* nodeE = graph.createNode<SinNode>();
        auto* two = graph.createNode<FloatConstantNode>(2.0f);

        // Connections
        graph.connect(nodeA->getOutput("Value"), nodeB->getInput("A"));
        graph.connect(nodeD->getOutput("Value"), nodeB->getInput("B"));
        graph.connect(nodeB->getOutput("Result"), nodeC->getInput("A"));
        graph.connect(two->getOutput("Value"), nodeC->getInput("B"));
        graph.connect(nodeC->getOutput("Result"), nodeE->getInput("Value"));

        REQUIRE(graph.getNodes().size() == 6);

        // Evaluate: (10 + 5) * 2 = 30, sin(30)
        nodeA->evaluate();
        nodeD->evaluate();
        two->evaluate();
        nodeB->evaluate();
        nodeC->evaluate();
        nodeE->evaluate();
        REQUIRE_THAT(std::get<float>(nodeB->getOutput("Result")->getValue()), WithinAbs(15.0f, 0.001f));
        REQUIRE_THAT(std::get<float>(nodeC->getOutput("Result")->getValue()), WithinAbs(30.0f, 0.001f));

        // Delete nodeC (middle node with both input and output connections)
        bool result = graph.deleteNode(nodeC->getId());

        REQUIRE(result == true);
        REQUIRE(graph.getNodes().size() == 5);

        // NodeB should still work
        nodeB->markDirty();
        nodeB->evaluate();
        REQUIRE_THAT(std::get<float>(nodeB->getOutput("Result")->getValue()), WithinAbs(15.0f, 0.001f));

        // NodeE should be disconnected
        REQUIRE_FALSE(nodeE->getInput("Value")->isConnected());

        // NodeE should use default (0), sin(0) = 0
        nodeE->markDirty();
        nodeE->evaluate();
        REQUIRE_THAT(std::get<float>(nodeE->getOutput("Result")->getValue()), WithinAbs(0.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 5: Graph Functionality After Node Deletion
// ====================================================================================
TEST_CASE("Graph continues to work correctly after node deletion", "[NodeGraph][NodeDeletion]") {
    NodeGraph graph;

    SECTION("Add new nodes after deletion and verify functionality") {
        // Create initial graph
        auto* node1 = graph.createNode<FloatConstantNode>(5.0f);
        auto* node2 = graph.createNode<FloatConstantNode>(3.0f);
        auto* addNode = graph.createNode<AddNode>();

        graph.connect(node1->getOutput("Value"), addNode->getInput("A"));
        graph.connect(node2->getOutput("Value"), addNode->getInput("B"));

        REQUIRE(graph.getNodes().size() == 3);

        // Evaluate: 5 + 3 = 8
        node1->evaluate();
        node2->evaluate();
        addNode->evaluate();
        REQUIRE_THAT(std::get<float>(addNode->getOutput("Result")->getValue()), WithinAbs(8.0f, 0.001f));

        // Delete one of the input nodes
        graph.deleteNode(node2->getId());
        REQUIRE(graph.getNodes().size() == 2);

        // Create a new node to replace it
        auto* node3 = graph.createNode<FloatConstantNode>(7.0f);
        graph.connect(node3->getOutput("Value"), addNode->getInput("B"));

        REQUIRE(graph.getNodes().size() == 3);

        // Evaluate new graph: 5 + 7 = 12
        node3->evaluate();
        addNode->markDirty();
        addNode->evaluate();
        REQUIRE_THAT(std::get<float>(addNode->getOutput("Result")->getValue()), WithinAbs(12.0f, 0.001f));
    }

    SECTION("Delete and rebuild entire graph") {
        // Create initial graph
        auto* a = graph.createNode<FloatConstantNode>(2.0f);
        auto* b = graph.createNode<FloatConstantNode>(3.0f);
        auto* mul = graph.createNode<MultiplyNode>();

        graph.connect(a->getOutput("Value"), mul->getInput("A"));
        graph.connect(b->getOutput("Value"), mul->getInput("B"));

        REQUIRE(graph.getNodes().size() == 3);

        // Delete all nodes
        graph.deleteNode(a->getId());
        graph.deleteNode(b->getId());
        graph.deleteNode(mul->getId());

        REQUIRE(graph.getNodes().size() == 0);

        // Build new graph
        auto* x = graph.createNode<FloatConstantNode>(10.0f);
        auto* y = graph.createNode<FloatConstantNode>(20.0f);
        auto* add = graph.createNode<AddNode>();

        graph.connect(x->getOutput("Value"), add->getInput("A"));
        graph.connect(y->getOutput("Value"), add->getInput("B"));

        REQUIRE(graph.getNodes().size() == 3);

        // Evaluate: 10 + 20 = 30
        x->evaluate();
        y->evaluate();
        add->evaluate();
        REQUIRE_THAT(std::get<float>(add->getOutput("Result")->getValue()), WithinAbs(30.0f, 0.001f));
    }

    SECTION("Partial deletion maintains remaining graph integrity") {
        // Build graph with two independent branches:
        // Branch 1: A(5) -> Add(A+B) <- B(3)
        // Branch 2: C(2) -> Mul(C*D) <- D(4)

        auto* a = graph.createNode<FloatConstantNode>(5.0f);
        auto* b = graph.createNode<FloatConstantNode>(3.0f);
        auto* addNode = graph.createNode<AddNode>();

        auto* c = graph.createNode<FloatConstantNode>(2.0f);
        auto* d = graph.createNode<FloatConstantNode>(4.0f);
        auto* mulNode = graph.createNode<MultiplyNode>();

        graph.connect(a->getOutput("Value"), addNode->getInput("A"));
        graph.connect(b->getOutput("Value"), addNode->getInput("B"));
        graph.connect(c->getOutput("Value"), mulNode->getInput("A"));
        graph.connect(d->getOutput("Value"), mulNode->getInput("B"));

        REQUIRE(graph.getNodes().size() == 6);

        // Evaluate both branches
        a->evaluate();
        b->evaluate();
        addNode->evaluate();
        REQUIRE_THAT(std::get<float>(addNode->getOutput("Result")->getValue()), WithinAbs(8.0f, 0.001f));

        c->evaluate();
        d->evaluate();
        mulNode->evaluate();
        REQUIRE_THAT(std::get<float>(mulNode->getOutput("Result")->getValue()), WithinAbs(8.0f, 0.001f));

        // Delete entire branch 1
        graph.deleteNode(a->getId());
        graph.deleteNode(b->getId());
        graph.deleteNode(addNode->getId());

        REQUIRE(graph.getNodes().size() == 3);

        // Branch 2 should still work correctly
        c->markDirty();
        d->markDirty();
        mulNode->markDirty();

        c->evaluate();
        d->evaluate();
        mulNode->evaluate();
        REQUIRE_THAT(std::get<float>(mulNode->getOutput("Result")->getValue()), WithinAbs(8.0f, 0.001f));
    }
}

// ====================================================================================
// TEST 6: Delete Multiple Connected Nodes
// ====================================================================================
TEST_CASE("Delete multiple connected nodes in sequence", "[NodeGraph][NodeDeletion]") {
    NodeGraph graph;

    SECTION("Delete nodes in dependency order") {
        // Create chain: A -> B -> C -> D
        auto* nodeA = graph.createNode<FloatConstantNode>(1.0f);
        auto* nodeB = graph.createNode<AddNode>();
        auto* nodeC = graph.createNode<MultiplyNode>();
        auto* nodeD = graph.createNode<SinNode>();

        auto* two = graph.createNode<FloatConstantNode>(2.0f);
        auto* three = graph.createNode<FloatConstantNode>(3.0f);

        graph.connect(nodeA->getOutput("Value"), nodeB->getInput("A"));
        graph.connect(two->getOutput("Value"), nodeB->getInput("B"));
        graph.connect(nodeB->getOutput("Result"), nodeC->getInput("A"));
        graph.connect(three->getOutput("Value"), nodeC->getInput("B"));
        graph.connect(nodeC->getOutput("Result"), nodeD->getInput("Value"));

        REQUIRE(graph.getNodes().size() == 6);

        // Delete from end to start (D -> C -> B)
        graph.deleteNode(nodeD->getId());
        REQUIRE(graph.getNodes().size() == 5);

        // nodeC should still work
        nodeA->evaluate();
        two->evaluate();
        three->evaluate();
        nodeB->evaluate();
        nodeC->evaluate();
        REQUIRE_THAT(std::get<float>(nodeC->getOutput("Result")->getValue()), WithinAbs(9.0f, 0.001f)); // (1+2)*3

        graph.deleteNode(nodeC->getId());
        REQUIRE(graph.getNodes().size() == 4);

        // nodeB should still work
        nodeB->markDirty();
        nodeB->evaluate();
        REQUIRE_THAT(std::get<float>(nodeB->getOutput("Result")->getValue()), WithinAbs(3.0f, 0.001f)); // 1+2

        graph.deleteNode(nodeB->getId());
        REQUIRE(graph.getNodes().size() == 3);

        // nodeA and constants should still work
        nodeA->markDirty();
        nodeA->evaluate();
        REQUIRE_THAT(std::get<float>(nodeA->getOutput("Value")->getValue()), WithinAbs(1.0f, 0.001f));
    }

    SECTION("Delete nodes in reverse dependency order") {
        // Create chain: A -> B -> C
        auto* nodeA = graph.createNode<FloatConstantNode>(5.0f);
        auto* nodeB = graph.createNode<MultiplyNode>();
        auto* nodeC = graph.createNode<AddNode>();

        auto* two = graph.createNode<FloatConstantNode>(2.0f);
        auto* ten = graph.createNode<FloatConstantNode>(10.0f);

        graph.connect(nodeA->getOutput("Value"), nodeB->getInput("A"));
        graph.connect(two->getOutput("Value"), nodeB->getInput("B"));
        graph.connect(nodeB->getOutput("Result"), nodeC->getInput("A"));
        graph.connect(ten->getOutput("Value"), nodeC->getInput("B"));

        REQUIRE(graph.getNodes().size() == 5);

        // Delete from start (A), middle node B should handle it gracefully
        graph.deleteNode(nodeA->getId());
        REQUIRE(graph.getNodes().size() == 4);

        REQUIRE_THAT(std::get<float>(nodeB->getOutput("Result")->getValue()), WithinAbs(2.0f, 0.001f)); // 1*2 // Default value of a multiple node is 1

        // nodeC should reflect the change
        nodeC->markDirty();
        nodeC->evaluate();
        REQUIRE_THAT(std::get<float>(nodeC->getOutput("Result")->getValue()), WithinAbs(12.0f, 0.001f)); // 1 + 10 // the default of an add node is 1
    }
}

// ====================================================================================
// TEST 7: Delete Node and Verify Memory Cleanup
// ====================================================================================
TEST_CASE("Verify proper cleanup after node deletion", "[NodeGraph][NodeDeletion]") {
    NodeGraph graph;

    SECTION("Node IDs remain valid after deletion") {
        auto* node1 = graph.createNode<FloatConstantNode>(1.0f);
        auto* node2 = graph.createNode<FloatConstantNode>(2.0f);
        auto* node3 = graph.createNode<FloatConstantNode>(3.0f);

        uint64_t id1 = node1->getId();
        uint64_t id2 = node2->getId();
        uint64_t id3 = node3->getId();

        REQUIRE(graph.getNodes().size() == 3);

        // Delete middle node
        graph.deleteNode(id2);
        REQUIRE(graph.getNodes().size() == 2);

        // Other nodes should still be accessible by their IDs
        bool found1 = false, found3 = false;
        for (const auto& node : graph.getNodes()) {
            if (node->getId() == id1) found1 = true;
            if (node->getId() == id3) found3 = true;
        }

        REQUIRE(found1);
        REQUIRE(found3);

        // Deleted node should not be found
        bool found2 = false;
        for (const auto& node : graph.getNodes()) {
            if (node->getId() == id2) found2 = true;
        }
        REQUIRE_FALSE(found2);
    }

    SECTION("New nodes get unique IDs after deletions") {
        auto* node1 = graph.createNode<FloatConstantNode>(1.0f);
        uint64_t id1 = node1->getId();

        graph.deleteNode(id1);

        auto* node2 = graph.createNode<FloatConstantNode>(2.0f);
        uint64_t id2 = node2->getId();

        // New node should have a different ID
        REQUIRE(id2 != id1);

        // Multiple new nodes should all have unique IDs
        auto* node3 = graph.createNode<FloatConstantNode>(3.0f);
        auto* node4 = graph.createNode<FloatConstantNode>(4.0f);

        REQUIRE(node3->getId() != node2->getId());
        REQUIRE(node4->getId() != node2->getId());
        REQUIRE(node4->getId() != node3->getId());
    }
}
