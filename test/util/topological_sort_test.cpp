#include <catch.hpp>
#include <algorithm>

#include <ophidian/util/GraphOperations.h>

#include "GraphFixture.h"

TEST_CASE_METHOD(DAGFixture, "Topological sort of a DAG with 4 nodes", "[util][topological_sort]") {
    std::vector<lemon::ListDigraph::Node> sortedNodes;
    ophidian::util::topologicalSort(mGraph, mNode1, sortedNodes);

    std::vector<lemon::ListDigraph::Node> expectedSortedNodes = {mNode1, mNode2, mNode3, mNode4};

    REQUIRE(sortedNodes.size() == expectedSortedNodes.size());
    REQUIRE(std::is_permutation(expectedSortedNodes.begin(), expectedSortedNodes.end(), sortedNodes.begin()));
}
