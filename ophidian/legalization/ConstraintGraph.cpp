#include "ConstraintGraph.h"

namespace ophidian {
namespace legalization {
ConstraintGraph::ConstraintGraph(design::Design & design)
    : mDesign(design),
      mCell2HorizontalNode(design.netlist().makeProperty<lemon::ListDigraph::Node>(circuit::Cell())), mCell2VerticalNode(design.netlist().makeProperty<lemon::ListDigraph::Node>(circuit::Cell())),
      mHorizontalNode2Cell(mHorizontalGraph), mVerticalNode2Cell(mVerticalGraph) {

}

void ConstraintGraph::buildConstraintGraph(const std::vector<circuit::Cell> &cells)
{
    for (auto cell : cells) {
        auto horizontalNode = mHorizontalGraph.addNode();
        mCell2HorizontalNode[cell] = horizontalNode;
        mHorizontalNode2Cell[horizontalNode] = cell;

        auto verticalNode = mVerticalGraph.addNode();
        mCell2VerticalNode[cell] = verticalNode;
        mVerticalNode2Cell[verticalNode] = cell;
    }

    for (auto cell1It = cells.begin(); cell1It != cells.end(); cell1It++) {
        auto cell2It = cell1It;
        cell2It++;
        for (cell2It; cell2It != cells.end(); cell2It++) {
            auto cell1 = *cell1It;
            auto cell2 = *cell2It;

            auto cell1Location = mDesign.placement().cellLocation(cell1).toPoint();
            auto cell1Box = mDesign.placementMapping().geometry(cell1)[0];
            auto cell1Width = cell1Box.max_corner().x() - cell1Box.min_corner().x();
            auto cell1Height = cell1Box.max_corner().y() - cell1Box.min_corner().y();

            auto cell2Location = mDesign.placement().cellLocation(cell2).toPoint();
            auto cell2Box = mDesign.placementMapping().geometry(cell2)[0];
            auto cell2Width = cell2Box.max_corner().x() - cell2Box.min_corner().x();
            auto cell2Height = cell2Box.max_corner().y() - cell2Box.min_corner().y();

            double xDistance = (cell1Location.x() <= cell2Location.x()) ? cell2Location.x() - (cell1Location.x() + cell1Width) : cell1Location.x() - (cell2Location.x() + cell2Width);
            double yDistance = (cell1Location.y() <= cell2Location.y()) ? cell2Location.y() - (cell1Location.y() + cell1Height) : cell1Location.y() - (cell2Location.y() + cell2Height);

            if (xDistance >= 0 && yDistance >= 0) {
                if (xDistance > yDistance) {
                    addEdge(mCell2HorizontalNode[cell1], mCell2HorizontalNode[cell2], cell1Location.x(), cell2Location.x(), mHorizontalGraph);
                } else {
                    addEdge(mCell2VerticalNode[cell1], mCell2VerticalNode[cell2], cell1Location.y(), cell2Location.y(), mVerticalGraph);
                }
            } else if (xDistance >= 0) {
                addEdge(mCell2HorizontalNode[cell1], mCell2HorizontalNode[cell2], cell1Location.x(), cell2Location.x(), mHorizontalGraph);
            } else if (yDistance >= 0) {
                addEdge(mCell2VerticalNode[cell1], mCell2VerticalNode[cell2], cell1Location.y(), cell2Location.y(), mVerticalGraph);
            } else {
                if (xDistance > yDistance) {
                    addEdge(mCell2HorizontalNode[cell1], mCell2HorizontalNode[cell2], cell1Location.x(), cell2Location.x(), mHorizontalGraph);
                } else {
                    addEdge(mCell2VerticalNode[cell1], mCell2VerticalNode[cell2], cell1Location.y(), cell2Location.y(), mVerticalGraph);
                }
            }
        }
    }

    util::transitiveClosure(mHorizontalGraph);
    util::transitiveClosure(mVerticalGraph);
    util::transitiveReduction(mHorizontalGraph);
    util::transitiveReduction(mVerticalGraph);
}

bool ConstraintGraph::leftRelationship(circuit::Cell cell1, circuit::Cell cell2)
{
    auto node1 = mCell2HorizontalNode[cell1];
    auto node2 = mCell2HorizontalNode[cell2];
    return lemon::findArc(mHorizontalGraph, node1, node2) != lemon::INVALID;
}

bool ConstraintGraph::rightRelationship(circuit::Cell cell1, circuit::Cell cell2)
{
    auto node1 = mCell2HorizontalNode[cell2];
    auto node2 = mCell2HorizontalNode[cell1];
    return lemon::findArc(mHorizontalGraph, node1, node2) != lemon::INVALID;
}

bool ConstraintGraph::topRelationship(circuit::Cell cell1, circuit::Cell cell2)
{
    auto node1 = mCell2VerticalNode[cell2];
    auto node2 = mCell2VerticalNode[cell1];
    return lemon::findArc(mVerticalGraph, node1, node2) != lemon::INVALID;
}

bool ConstraintGraph::bottomRelationship(circuit::Cell cell1, circuit::Cell cell2)
{
    auto node1 = mCell2VerticalNode[cell1];
    auto node2 = mCell2VerticalNode[cell2];
    return lemon::findArc(mVerticalGraph, node1, node2) != lemon::INVALID;
}

void ConstraintGraph::addEdge(lemon::ListDigraph::Node node1, lemon::ListDigraph::Node node2, double cell1Coordinate, double cell2Coordinate, lemon::ListDigraph &graph)
{
    if (cell1Coordinate < cell2Coordinate) {
        graph.addArc(node1, node2);
    } else {
        graph.addArc(node2, node1);
    }
}

}
}
