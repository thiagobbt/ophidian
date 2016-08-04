#include "delay_capacitance_map.h"

namespace ophidian {
namespace clock_tree_synthesis {

delay_capacitance_map::delay_capacitance_map(const graph_t &graph)
    :m_delays(graph), m_capacitances(graph){

}
delay_capacitance_map::~delay_capacitance_map()
{

}


void delay_capacitance_map::node_delay(delay_capacitance_map::node graph_node, double delay)
{
    m_delays[graph_node] = delay;
}


void delay_capacitance_map::node_capacitance(delay_capacitance_map::node graph_node, double capacitance)
{
    m_capacitances[graph_node] = capacitance;
}

}
}
