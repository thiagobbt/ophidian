#ifndef DELAY_CAPACITANCE_MAP_H
#define DELAY_CAPACITANCE_MAP_H

#include <lemon/list_graph.h>

#include "../geometry/geometry.h"

namespace ophidian {
namespace clock_tree_synthesis {


class delay_capacitance_map
{
public:
    using point = geometry::point<double>;
    using graph_t = lemon::ListDigraph;
    using node = graph_t::Node;
private:
    graph_t::NodeMap<double> m_delays;
    graph_t::NodeMap<double> m_capacitances;

public:
    delay_capacitance_map(const graph_t &graph);
    ~delay_capacitance_map();

    double node_delay(node graph_node) const {
        return m_delays[graph_node];
    }
    void node_delay(node graph_node, double delay);

    double node_capacitance(node graph_node) const {
        return m_capacitances[graph_node];
    }
    void node_capacitance(node graph_node, double capacitance);
};

}
}
#endif // DELAY_CAPACITANCE_MAP_H
