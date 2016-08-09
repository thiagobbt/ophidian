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

#ifndef CLOCK_TOPOLOGY_H
#define CLOCK_TOPOLOGY_H

#include <lemon/list_graph.h>

#include "../geometry/geometry.h"

namespace ophidian {
namespace clock_tree_synthesis {

template <class PointType>
class clock_topology
{
public:
    using point = geometry::point<PointType>;
    using graph_t = lemon::ListDigraph;
    using node = graph_t::Node;
    using edge = graph_t::Arc;
private:
    graph_t m_graph;

public:

    clock_topology(){}
    ~clock_topology(){}

    node node_create()
    {
        auto node = m_graph.addNode();;
        return node;
    }


    std::size_t nodes_count() const {
        return lemon::countNodes(m_graph);
    }

    void node_children(node graph_node, std::vector<node> &children)
    {
        for(auto arc_it = graph_t::OutArcIt(m_graph, graph_node); arc_it != lemon::INVALID; ++arc_it) {
            children.push_back(m_graph.target(arc_it));
        }
    }

    node node_parent(node graph_node)
    {
        std::vector<node> source_nodes;
        for(auto arc_it = graph_t::InArcIt(m_graph, graph_node); arc_it != lemon::INVALID; ++arc_it) {
            source_nodes.push_back(m_graph.source(arc_it));
        }
        assert(source_nodes.size() == 1);
        return source_nodes.front();
    }

    edge edge_create(node source, node target)
    {
        auto edge = m_graph.addArc(source, target);
        return edge;
    }

    const graph_t & graph() const {
        return m_graph;
    }

};
}
}

#endif // CLOCK_TOPOLOGY_H
