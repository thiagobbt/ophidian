#include "embedding.h"

namespace ophidian {
namespace clock_tree_synthesis {
embedding::embedding(const graph_t& graph)
    : m_positions(graph){

}

embedding::~embedding()
{

}

void embedding::node_position(embedding::node graph_node, embedding::point position)
{
    m_positions[graph_node] = position;
}

}
}
