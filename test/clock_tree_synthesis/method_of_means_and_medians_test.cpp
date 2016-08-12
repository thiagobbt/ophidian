
/*
 * Copyright 2016 Ophidian
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
 */

#include "../catch.hpp"
#include "method_of_means_and_medians.h"
#include "embedding.h"
#include "delay_capacitance_map.h"

#include <lemon/list_graph.h>
#include <lemon/dfs.h>

bool point_comparison(const ophidian::geometry::point<double> & point1, const ophidian::geometry::point<double> & point2) {
    return boost::geometry::equals(point1, point2);
}

TEST_CASE("method of means and medians/ build test graph","[mmm]") {
    std::vector<ophidian::geometry::point<double>> flip_flop_positions = {
        {1, 1},
        {3, 1},
        {5, 1},
        {7, 1},
        {1, 3},
        {3, 3},
        {5, 3},
        {7, 3},
        {1, 5},
        {3, 5},
        {5, 5},
        {7, 5},
        {1, 7},
        {3, 7},
        {5, 7},
        {7, 7}
    };
    ophidian::geometry::point<double> clock_source(4, 0);
    ophidian::clock_tree_synthesis::method_of_means_and_medians mmm;
    ophidian::clock_tree_synthesis::topology clock_topology;
    ophidian::clock_tree_synthesis::embedding embedding(clock_topology.graph());
    mmm.build_topology(clock_source, flip_flop_positions, clock_topology, embedding);
    mmm.build(clock_topology, embedding);

    std::vector<ophidian::geometry::point<double>> expected_node_positions = {
        {1, 1},
        {3, 1},
        {5, 1},
        {7, 1},
        {1, 3},
        {3, 3},
        {5, 3},
        {7, 3},
        {1, 5},
        {3, 5},
        {5, 5},
        {7, 5},
        {1, 7},
        {3, 7},
        {5, 7},
        {7, 7},
        {2, 2},
        {2, 4},
        {2, 6},
        {6, 2},
        {6, 4},
        {6, 6},
        {4, 4},
        {1, 2},
        {1, 6},
        {3, 2},
        {3, 6},
        {5, 2},
        {5, 6},
        {7, 2},
        {7, 6},
        {4, 0}
    };
    REQUIRE(clock_topology.nodes_count() == expected_node_positions.size());
    std::vector<ophidian::geometry::point<double>> final_node_positions;
    final_node_positions.reserve(lemon::countNodes(clock_topology.graph()));
    for (auto node_it = lemon::ListDigraph::NodeIt(clock_topology.graph()); node_it != lemon::INVALID; ++node_it) {
        final_node_positions.push_back(embedding.node_position(node_it));
    }
    REQUIRE(std::is_permutation(final_node_positions.begin(), final_node_positions.end(), expected_node_positions.begin(), point_comparison));
}

TEST_CASE("method of means and medians/ build topology of network with 3 flip flops","[mmm]") {
    std::vector<ophidian::geometry::point<double>> flip_flop_positions = {
        {1, 1},
        {1, 3},
        {5, 3}
    };
    ophidian::geometry::point<double> clock_source(3, 0);
    ophidian::clock_tree_synthesis::method_of_means_and_medians mmm;
    ophidian::clock_tree_synthesis::topology clock_topology;
    ophidian::clock_tree_synthesis::embedding embedding(clock_topology.graph());
    mmm.build_topology(clock_source, flip_flop_positions, clock_topology, embedding);
    mmm.build(clock_topology, embedding);

    std::vector<ophidian::geometry::point<double>> expected_node_positions = {
        {1, 1},
        {1, 3},
        {5, 3},
        {3, 0},
        {7.0/3.0, 7.0/3.0},
        {1, 2}
    };
    REQUIRE(clock_topology.nodes_count() == expected_node_positions.size());
    std::vector<ophidian::geometry::point<double>> final_node_positions;
    final_node_positions.reserve(lemon::countNodes(clock_topology.graph()));
    for (auto node_it = lemon::ListDigraph::NodeIt(clock_topology.graph()); node_it != lemon::INVALID; ++node_it) {
        final_node_positions.push_back(embedding.node_position(node_it));
    }
    REQUIRE(std::is_permutation(final_node_positions.begin(), final_node_positions.end(), expected_node_positions.begin(), point_comparison));
}

TEST_CASE("method of means and medians/ build topology of network with 4 flip flops","[mmm]") {
    std::vector<ophidian::geometry::point<double>> flip_flop_positions = {
        {1, 1},
        {3, 1},
        {1, 3},
        {3, 3}
    };
    ophidian::geometry::point<double> clock_source(2, 0);
    ophidian::clock_tree_synthesis::method_of_means_and_medians mmm;
    ophidian::clock_tree_synthesis::topology clock_topology;
    ophidian::clock_tree_synthesis::embedding embedding(clock_topology.graph());
    mmm.build_topology(clock_source, flip_flop_positions, clock_topology, embedding);
    mmm.build(clock_topology, embedding);

    std::vector<ophidian::geometry::point<double>> expected_node_positions = {
        {1, 1},
        {3, 1},
        {1, 3},
        {3, 3},
        {2, 0},
        {1, 2},
        {2, 2},
        {3, 2}
    };
    REQUIRE(clock_topology.nodes_count() == expected_node_positions.size());
    std::vector<ophidian::geometry::point<double>> final_node_positions;
    final_node_positions.reserve(lemon::countNodes(clock_topology.graph()));
    for (auto node_it = lemon::ListDigraph::NodeIt(clock_topology.graph()); node_it != lemon::INVALID; ++node_it) {
        final_node_positions.push_back(embedding.node_position(node_it));
    }
    REQUIRE(std::is_permutation(final_node_positions.begin(), final_node_positions.end(), expected_node_positions.begin(), point_comparison));
}

TEST_CASE("method of means and medians/ build topology of network with 8 flip flops","[mmm]") {
    std::vector<ophidian::geometry::point<double>> flip_flop_positions = {
        {1, 1},
        {3, 2},
        {7, 2},
        {7, 1},
        {1, 9},
        {3, 7},
        {7, 7},
        {7, 9}
    };
    ophidian::geometry::point<double> clock_source(5, 0);
    ophidian::clock_tree_synthesis::method_of_means_and_medians mmm;
    ophidian::clock_tree_synthesis::topology clock_topology;
    ophidian::clock_tree_synthesis::embedding embedding(clock_topology.graph());
    mmm.build_topology(clock_source, flip_flop_positions, clock_topology, embedding);
    mmm.build(clock_topology, embedding);

    std::vector<ophidian::geometry::point<double>> expected_node_positions = {
        {1, 1},
        {3, 2},
        {7, 2},
        {7, 1},
        {1, 9},
        {3, 7},
        {7, 7},
        {7, 9},
        {5, 0},
        {4.5, 4.75},
        {2, 4.75},
        {7, 4.75},
        {2, 1.5},
        {7, 1.5},
        {2, 8},
        {7, 8}
    };
    REQUIRE(clock_topology.nodes_count() == expected_node_positions.size());
    std::vector<ophidian::geometry::point<double>> final_node_positions;
    final_node_positions.reserve(lemon::countNodes(clock_topology.graph()));
    for (auto node_it = lemon::ListDigraph::NodeIt(clock_topology.graph()); node_it != lemon::INVALID; ++node_it) {
        final_node_positions.push_back(embedding.node_position(node_it));
    }
    REQUIRE(std::is_permutation(final_node_positions.begin(), final_node_positions.end(), expected_node_positions.begin(), point_comparison));
}

TEST_CASE("method of means and medians/ build topology of network with 6 flip flops","[mmm]") {
    std::vector<ophidian::geometry::point<double>> flip_flop_positions = {
        {1, 1},
        {3, 2},
        {7, 2},
        {8, 1},
        {1, 9},
        {7, 9}
    };
    ophidian::geometry::point<double> clock_source(5, 0);
    ophidian::clock_tree_synthesis::method_of_means_and_medians mmm;
    ophidian::clock_tree_synthesis::topology clock_topology;
    ophidian::clock_tree_synthesis::embedding embedding(clock_topology.graph());
    mmm.build_topology(clock_source, flip_flop_positions, clock_topology, embedding);
    mmm.build(clock_topology, embedding);

    std::vector<ophidian::geometry::point<double>> expected_node_positions = {
        {1, 1},
        {3, 2},
        {7, 2},
        {8, 1},
        {1, 9},
        {7, 9},
        {5, 0},
        {4.5, 4},
        {5.0/3.0, 4},
        {22.0/3.0, 4},
        {2, 1.5},
        {7.5, 1.5}
    };
    REQUIRE(clock_topology.nodes_count() == expected_node_positions.size());
    std::vector<ophidian::geometry::point<double>> final_node_positions;
    final_node_positions.reserve(lemon::countNodes(clock_topology.graph()));
    for (auto node_it = lemon::ListDigraph::NodeIt(clock_topology.graph()); node_it != lemon::INVALID; ++node_it) {
        final_node_positions.push_back(embedding.node_position(node_it));
    }
    REQUIRE(std::is_permutation(final_node_positions.begin(), final_node_positions.end(), expected_node_positions.begin(), point_comparison));
}

TEST_CASE("method of means and medians/ build topology of network with 9 flip flops","[mmm]") {
    std::vector<ophidian::geometry::point<double>> flip_flop_positions = {
        {1, 1},
        {3, 2},
        {7, 2},
        {7, 1},
        {1, 9},
        {3, 7},
        {7, 7},
        {7, 9},
        {2, 1}
    };
    ophidian::geometry::point<double> clock_source(5, 0);
    ophidian::clock_tree_synthesis::method_of_means_and_medians mmm;
    ophidian::clock_tree_synthesis::topology clock_topology;
    ophidian::clock_tree_synthesis::embedding embedding(clock_topology.graph());
    mmm.build_topology(clock_source, flip_flop_positions, clock_topology, embedding);
    mmm.build(clock_topology, embedding);

    std::vector<ophidian::geometry::point<double>> expected_node_positions = {
        {1, 1},
        {3, 2},
        {7, 2},
        {7, 1},
        {1, 9},
        {3, 7},
        {7, 7},
        {7, 9},
        {2, 1},
        {5, 0},
        {38.0/9.0, 39.0/9.0},
        {2, 4},
        {7, 4.75},
        {2, 4.0/3.0},
        {1.5, 1},
        {7, 1.5},
        {2, 8},
        {7, 8}
    };
    REQUIRE(clock_topology.nodes_count() == expected_node_positions.size());
    std::vector<ophidian::geometry::point<double>> final_node_positions;
    final_node_positions.reserve(lemon::countNodes(clock_topology.graph()));
    for (auto node_it = lemon::ListDigraph::NodeIt(clock_topology.graph()); node_it != lemon::INVALID; ++node_it) {
        final_node_positions.push_back(embedding.node_position(node_it));
    }
    REQUIRE(std::is_permutation(final_node_positions.begin(), final_node_positions.end(), expected_node_positions.begin(), point_comparison));
}

