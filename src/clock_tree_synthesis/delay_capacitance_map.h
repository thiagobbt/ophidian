#ifndef DELAY_CAPACITANCE_MAP_H
#define DELAY_CAPACITANCE_MAP_H

#include <lemon/list_graph.h>

#include "../geometry/geometry.h"

namespace ophidian {
namespace clock_tree_synthesis {


class delay_capacitance_map
{
public:
    using graph_t = lemon::ListDigraph;
    using node = graph_t::Node;
private:
    graph_t::NodeMap<double> m_delays;
    graph_t::NodeMap<double> m_capacitances;

public:
    delay_capacitance_map(const graph_t &graph)
        :m_delays(graph), m_capacitances(graph){

    }
    ~delay_capacitance_map()
    {

    }

    double node_delay(node graph_node) const {
        return m_delays[graph_node];
    }
    void node_delay(node graph_node, double delay)
    {
        m_delays[graph_node] = delay;
    }

    double node_capacitance(node graph_node) const {
        return m_capacitances[graph_node];
    }
    void node_capacitance(node graph_node, double capacitance)
    {
        m_capacitances[graph_node] = capacitance;
    }
};

}
}
#endif // DELAY_CAPACITANCE_MAP_H
