#ifndef EMBEDDING_H
#define EMBEDDING_H

#include <lemon/list_graph.h>

#include "../geometry/geometry.h"

namespace ophidian {
namespace clock_tree_synthesis {

template <class PointType>
class embedding
{
public:
    using point = geometry::point<PointType>;
    using graph_t = lemon::ListDigraph;
    using node = graph_t::Node;
private:
    graph_t::NodeMap<point> m_positions;

public:
    embedding(const graph_t &graph) : m_positions(graph)
    {

    }
    ~embedding()
    {

    }

    point node_position(node graph_node) const {
        return m_positions[graph_node];
    }

    void node_position(embedding::node graph_node, embedding::point position)
    {
        m_positions[graph_node] = position;
    }


};
}
}
#endif // EMBEDDING_H
