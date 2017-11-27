#ifndef CONSTRAINTGRAPH_H
#define CONSTRAINTGRAPH_H

#include <lemon/list_graph.h>

#include <ophidian/util/GraphOperations.h>

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {
class ConstraintGraph
{
public:
    ConstraintGraph(design::Design & design);

    void buildConstraintGraph(const std::vector<circuit::Cell> & cells);

    bool leftRelationship(circuit::Cell cell1, circuit::Cell cell2);

    bool rightRelationship(circuit::Cell cell1, circuit::Cell cell2);

    bool topRelationship(circuit::Cell cell1, circuit::Cell cell2);

    bool bottomRelationship(circuit::Cell cell1, circuit::Cell cell2);

private:
    void addEdge(lemon::ListDigraph::Node node1, lemon::ListDigraph::Node node2, double cell1Coordinate, double cell2Coordinate, lemon::ListDigraph & graph);

    design::Design & mDesign;

    lemon::ListDigraph mHorizontalGraph;
    lemon::ListDigraph mVerticalGraph;

    entity_system::Property<circuit::Cell, lemon::ListDigraph::Node> mCell2HorizontalNode;
    entity_system::Property<circuit::Cell, lemon::ListDigraph::Node> mCell2VerticalNode;
    lemon::ListDigraph::NodeMap<circuit::Cell> mHorizontalNode2Cell;
    lemon::ListDigraph::NodeMap<circuit::Cell> mVerticalNode2Cell;
};
}
}


#endif // CONSTRAINTGRAPH_H
