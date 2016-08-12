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


#include "method_of_means_and_medians.h"
#include <lemon/list_graph.h>
//#include <lemon/dfs.h>
#include <lemon/connectivity.h>

namespace ophidian {
namespace clock_tree_synthesis {
method_of_means_and_medians::method_of_means_and_medians()
{

}

method_of_means_and_medians::~method_of_means_and_medians()
{

}


void method_of_means_and_medians::build_topology(const method_of_means_and_medians::point &clock_source, const std::vector<method_of_means_and_medians::point> &flip_flop_positions, topology &clock_topology, embedding & embedding_for_sinks)
{
    std::vector<point> flip_flop_positions_copy = flip_flop_positions;
    auto source_node = clock_topology.node_create();
    embedding_for_sinks.node_position(source_node, clock_source);
    build_topology_internal(source_node, flip_flop_positions_copy.begin(), flip_flop_positions_copy.end(), clock_topology, embedding_for_sinks, true);
}

void method_of_means_and_medians::build_topology_internal(topology::node parent_node, std::vector<point>::iterator positions_begin, std::vector<point>::iterator positions_end, topology &clock_topology, embedding &embedding_for_sinks, bool sort_by_x_coordinate)
{
    auto node = clock_topology.node_create();
    clock_topology.edge_create(parent_node, node);
    auto size = std::distance(positions_begin, positions_end);
    if (size > 1) {
        std::sort(positions_begin, positions_end, coordinate_comparator(sort_by_x_coordinate));
        auto positions_middle = std::next(positions_begin, std::ceil(size / 2.0));
        build_topology_internal(node, positions_begin, positions_middle, clock_topology, embedding_for_sinks, !sort_by_x_coordinate);
        build_topology_internal(node, positions_middle, positions_end, clock_topology, embedding_for_sinks, !sort_by_x_coordinate);
    }else{
        embedding_for_sinks.node_position(node, *positions_begin);
    }
}

void method_of_means_and_medians::build(const topology &clock_topology, embedding &embedding)
{
    std::vector<topology::node> sorted_nodes(clock_topology.nodes_count());
    topology::graph_t::NodeMap<unsigned> order(clock_topology.graph());
    lemon::topologicalSort(clock_topology.graph(), order);
    for (auto node_it = topology::graph_t::NodeIt(clock_topology.graph()); node_it != lemon::INVALID; ++node_it) {
        sorted_nodes[order[node_it]] = node_it;
    }
    topology::graph_t::NodeMap<point> acumulated_sum_of_sinks_positions(clock_topology.graph());
    topology::graph_t::NodeMap<unsigned> number_of_visited_sinks(clock_topology.graph());
    for (auto node_it = sorted_nodes.rbegin(); node_it != sorted_nodes.rend(); ++node_it) {
        std::size_t number_of_out_arcs = lemon::countOutArcs(clock_topology.graph(), *node_it);
        std::size_t number_of_in_arcs = lemon::countInArcs(clock_topology.graph(), *node_it);
        if (number_of_out_arcs != 0 && number_of_in_arcs != 0) {
            //intermediate node
            point center_of_mass{0.0, 0.0};
            int number_of_sinks = 0;
            for (topology::graph_t::OutArcIt a(clock_topology.graph(), *node_it); a != lemon::INVALID; ++a){
                topology::graph_t::Node child = clock_topology.graph().target(a);
                point position_child = acumulated_sum_of_sinks_positions[child];
                number_of_sinks += number_of_visited_sinks[child];
                center_of_mass.x(center_of_mass.x() + position_child.x());
                center_of_mass.y(center_of_mass.y() + position_child.y());
            }
            acumulated_sum_of_sinks_positions[*node_it] = center_of_mass;
            number_of_visited_sinks[*node_it] = number_of_sinks;
            center_of_mass.x(center_of_mass.x() / (double)number_of_sinks);
            center_of_mass.y(center_of_mass.y() / (double)number_of_sinks);
            embedding.node_position(*node_it, center_of_mass);
        }else if(number_of_out_arcs == 0){
            //leaf
            acumulated_sum_of_sinks_positions[*node_it] = embedding.node_position(*node_it);
            number_of_visited_sinks[*node_it] = 1;
        }
    }
}


}
}


