#ifndef CONSTRAINTGRAPH_H
#define CONSTRAINTGRAPH_H

#include <lemon/list_graph.h>

#include <ophidian/util/GraphOperations.h>

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {

class LeftComparator {
public:
    bool operator()(util::Location & cell1Location, util::Location & cell2Location, geometry::Box & cell1Box, geometry::Box & cell2Box);

    double arcCost(geometry::Box & cell1Box);
};

class BelowComparator {
public:
    bool operator()(util::Location & cell1Location, util::Location & cell2Location, geometry::Box & cell1Box, geometry::Box & cell2Box);

    double arcCost(geometry::Box & cell1Box);
};

template <class ComparatorType>
class ConstraintGraph
{
public:
    ConstraintGraph(design::Design & design)
        : mDesign(design),
          mCell2Node(design.netlist().makeProperty<lemon::ListDigraph::Node>(circuit::Cell())),
          mNode2Cell(mGraph), mArcCosts(mGraph),
          mMinimumLocations(mGraph), mMaximumLocations(mGraph)
    {

    }

    void buildConstraintGraph(const std::vector<circuit::Cell> & cells) {
        ComparatorType comparator;

        for (auto cell : cells) {
            auto node = mGraph.addNode();
            mCell2Node[cell] = node;
            mNode2Cell[node] = cell;
        }

        for (auto cell1 : cells) {
            for (auto cell2 : cells) {
                auto cell1Location = mDesign.placement().cellLocation(cell1);
                auto cell1Box = mDesign.placementMapping().geometry(cell1)[0];

                auto cell2Location = mDesign.placement().cellLocation(cell2);
                auto cell2Box = mDesign.placementMapping().geometry(cell2)[0];

                if (comparator(cell1Location, cell2Location, cell1Box, cell2Box) && !hasEdge(cell2, cell1)) {
                    auto arc = mGraph.addArc(mCell2Node[cell1], mCell2Node[cell2]);
                    mArcCosts[arc] = comparator.arcCost(cell1Box);
                }
            }
        }
    }

    void calculateSlacks() {
        auto source = mGraph.addNode();
        auto sink = mGraph.addNode();

        for (auto node = lemon::ListDigraph::NodeIt(mGraph); node != lemon::INVALID; ++node) {
            if (lemon::countInArcs(mGraph, node)) {
                auto arc = mGraph.addArc(source, node);
                mArcCosts[arc] = 0;
            } else if (lemon::countOutArcs(mGraph, node)) {
                auto arc = mGraph.addArc(node, sink);
                mArcCosts[arc] = 0;
            }
        }

    }

    void removeTransitiveEdges() {
        util::transitiveClosure(mGraph);
        util::transitiveReduction(mGraph);
    }

    bool hasEdge(circuit::Cell cell1, circuit::Cell cell2) {
        auto node1 = mCell2Node[cell1];
        auto node2 = mCell2Node[cell2];
        return lemon::findArc(mGraph, node1, node2) != lemon::INVALID;
    }

    const lemon::ListDigraph & graph() {
        return mGraph;
    }

protected:
    lemon::ListDigraph mGraph;

private:
    design::Design & mDesign;

    entity_system::Property<circuit::Cell, lemon::ListDigraph::Node> mCell2Node;
    lemon::ListDigraph::NodeMap<circuit::Cell> mNode2Cell;

    lemon::ListDigraph::ArcMap<double> mArcCosts;

    lemon::ListDigraph::NodeMap<util::Location> mMinimumLocations;
    lemon::ListDigraph::NodeMap<util::Location> mMaximumLocations;
};
}
}


#endif // CONSTRAINTGRAPH_H
