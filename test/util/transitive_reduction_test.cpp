#include <catch.hpp>

#include <ophidian/util/GraphOperations.h>

#include "GraphFixture.h"

TEST_CASE_METHOD(TransitiveGraphFixture, "Transitive reduction of a graph with 4 nodes", "[util][transitive_reduction]") {
    ophidian::util::transitiveReduction(mGraph);

    SECTION("Out arcs of node1") {
        REQUIRE(lemon::findArc(mGraph, mNode1, mNode2) == lemon::INVALID);
        REQUIRE(lemon::findArc(mGraph, mNode1, mNode3) != lemon::INVALID);
        REQUIRE(lemon::findArc(mGraph, mNode1, mNode4) != lemon::INVALID);
    }

    SECTION("Out arcs of node2") {
        REQUIRE(lemon::findArc(mGraph, mNode2, mNode1) != lemon::INVALID);
        REQUIRE(lemon::findArc(mGraph, mNode2, mNode3) == lemon::INVALID);
        REQUIRE(lemon::findArc(mGraph, mNode2, mNode4) == lemon::INVALID);
    }

    SECTION("Out arcs of node3") {
        REQUIRE(lemon::findArc(mGraph, mNode3, mNode1) == lemon::INVALID);
        REQUIRE(lemon::findArc(mGraph, mNode3, mNode2) == lemon::INVALID);
        REQUIRE(lemon::findArc(mGraph, mNode3, mNode4) == lemon::INVALID);
    }

    SECTION("Out arcs of node4") {
        REQUIRE(lemon::findArc(mGraph, mNode4, mNode1) == lemon::INVALID);
        REQUIRE(lemon::findArc(mGraph, mNode4, mNode2) != lemon::INVALID);
        REQUIRE(lemon::findArc(mGraph, mNode4, mNode3) == lemon::INVALID);
    }
}
