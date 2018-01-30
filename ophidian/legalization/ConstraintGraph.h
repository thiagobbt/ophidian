#ifndef CONSTRAINTGRAPH_H
#define CONSTRAINTGRAPH_H

#include <fstream>

#include <lemon/list_graph.h>
#include <lemon/preflow.h>

#include <ophidian/util/GraphOperations.h>

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {

class LeftComparator {
public:
    bool operator()(util::Location & cell1Location, util::Location & cell2Location, geometry::Box & cell1Box, geometry::Box & cell2Box);

    bool operator()(util::Location & cell1Location, util::Location & cell2Location);

    double arcCost(geometry::Box & cell1Box);
};

class BelowComparator {
public:
    bool operator()(util::Location & cell1Location, util::Location & cell2Location, geometry::Box & cell1Box, geometry::Box & cell2Box);

    bool operator()(util::Location & cell1Location, util::Location & cell2Location);

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
        mSource = lemon::INVALID;
        mSink = lemon::INVALID;
    }

    void buildConstraintGraph(const std::vector<circuit::Cell> & cells, util::micrometer_t min, util::micrometer_t max) {
        ComparatorType comparator;

        for (auto cell : cells) {
            auto node = mGraph.addNode();
            mCell2Node[cell] = node;
            mNode2Cell[node] = cell;
        }

        for (auto cell1 : cells) {
            for (auto cell2 : cells) {
                if (cell1 != cell2) {
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

        calculateSlacks(units::unit_cast<double>(min), units::unit_cast<double>(max));
    }

    void calculateSlacks(double min, double max) {
        ComparatorType comparator;

        if (mSource != lemon::INVALID) {
            mGraph.erase(mSource);
        }
        if (mSink != lemon::INVALID) {
            mGraph.erase(mSink);
        }

        mSource = mGraph.addNode();
        mSink = mGraph.addNode();

        for (auto node = lemon::ListDigraph::NodeIt(mGraph); node != lemon::INVALID; ++node) {
            if (node == mSource || node == mSink) {
                continue;
            }
            if (lemon::countInArcs(mGraph, node) == 0) {
                auto arc = mGraph.addArc(mSource, node);
                mArcCosts[arc] = 0;
            }
            if (lemon::countOutArcs(mGraph, node) == 0) {
                auto cell = mNode2Cell[node];
                auto cellBox = mDesign.placementMapping().geometry(cell)[0];

                auto arc = mGraph.addArc(node, mSink);
                mArcCosts[arc] = comparator.arcCost(cellBox);
            }
        }

        std::vector<lemon::ListDigraph::Node> sortedNodes;
        util::topologicalSort(mGraph, mSource, sortedNodes);

        mMinimumLocations[mSource] = min;
        mMaximumLocations[mSink] = max;
        for (auto node : sortedNodes) {
            mMinimumLocations[node] = min;
            for (auto arc = lemon::ListDigraph::InArcIt(mGraph, node); arc != lemon::INVALID; ++arc) {
                auto source = mGraph.source(arc);
                mMinimumLocations[node] = std::max(mMinimumLocations[node], mMinimumLocations[source] + mArcCosts[arc]);
            }
        }

        for (auto nodeIt = sortedNodes.rbegin(); nodeIt != sortedNodes.rend(); nodeIt++) {
            auto node = *nodeIt;
            mMaximumLocations[node] = max;
            for (auto arc = lemon::ListDigraph::OutArcIt(mGraph, node); arc != lemon::INVALID; ++arc) {
                auto target = mGraph.target(arc);
                mMaximumLocations[node] = std::min(mMaximumLocations[node], mMaximumLocations[target] - mArcCosts[arc]);
            }
        }
    }

    double minimumLocation(circuit::Cell cell) {
        auto node = mCell2Node[cell];
        return mMinimumLocations[node];
    }

    double maximumLocation(circuit::Cell cell) {
        auto node = mCell2Node[cell];
        return mMaximumLocations[node];
    }

    double slack(circuit::Cell cell) {
        auto node = mCell2Node[cell];
        return mMaximumLocations[node] - mMinimumLocations[node];
    }

    double worstSlack() {
        double worstSlack = std::numeric_limits<double>::max();
        for (auto node = lemon::ListDigraph::NodeIt(mGraph); node != lemon::INVALID; ++node) {
            if (node != mSource && node != mSink) {
                auto cell = mNode2Cell[node];
                auto cellSlack = slack(cell);
                worstSlack = std::min(worstSlack, cellSlack);
            }
        }
        return worstSlack;
    }

    bool isFeasible() {
        for (auto node = lemon::ListDigraph::NodeIt(mGraph); node != lemon::INVALID; ++node) {
            if (node != mSource && node != mSink) {
                auto cell = mNode2Cell[node];
                auto cellSlack = slack(cell);
                if (cellSlack < 0) {
                    return false;
                }
            }
        }
        return true;
    }

    bool hasEdge(circuit::Cell cell1, circuit::Cell cell2) {
        auto node1 = mCell2Node[cell1];
        auto node2 = mCell2Node[cell2];
        return lemon::findArc(mGraph, node1, node2) != lemon::INVALID;
    }

    void addEdge(circuit::Cell cell1, circuit::Cell cell2) {
        auto node1 = mCell2Node[cell1];
        auto node2 = mCell2Node[cell2];
        auto arc = mGraph.addArc(node1, node2);

        ComparatorType comparator;
        auto cell1Box = mDesign.placementMapping().geometry(cell1)[0];
        auto cost = comparator.arcCost(cell1Box);
        mArcCosts[arc] = cost;
    }

    const lemon::ListDigraph & graph() {
        return mGraph;
    }

    template <typename OrthogonalComparatorType>
    void adjustGraph(ConstraintGraph<OrthogonalComparatorType> & orthogonalGraph, util::micrometer_t min, util::micrometer_t max, util::micrometer_t orthogonalMin, util::micrometer_t orthogonalMax) {
        OrthogonalComparatorType orthogonalComparator;
        lemon::ListDigraph::ArcMap<double> capacityMap(mGraph);

        double bestSlack = std::numeric_limits<double>::max();
        for (auto arc = lemon::ListDigraph::ArcIt(mGraph); arc != lemon::INVALID; ++arc) {
            auto arcSource = mGraph.source(arc);
            auto arcTarget = mGraph.target(arc);

            if (arcSource == mSource || arcTarget == mSink) {
                capacityMap[arc] = std::numeric_limits<double>::max();
                continue;
            }

            auto sourceCell = mNode2Cell[arcSource];
            auto sourceBox = mDesign.placementMapping().geometry(sourceCell)[0];
            auto sourceSize = orthogonalComparator.arcCost(sourceBox);

            auto targetCell = mNode2Cell[arcTarget];


            auto sourceMinimumOrthogonalLocation = orthogonalGraph.minimumLocation(sourceCell);
            auto sourceMaximumOrthogonalLocation = orthogonalGraph.maximumLocation(sourceCell);
            auto targetMaximumOrthogonalLocation = orthogonalGraph.maximumLocation(targetCell);

            double newSourceMaximumLocation = std::min(sourceMaximumOrthogonalLocation, targetMaximumOrthogonalLocation - sourceSize);
            double newSlack = newSourceMaximumLocation - sourceMinimumOrthogonalLocation;
            if (newSlack < 0) {
                capacityMap[arc] = std::numeric_limits<double>::max();
            } else {
                capacityMap[arc] = -newSlack;
                bestSlack = std::min(bestSlack, -newSlack);
            }
        }

        if (bestSlack != std::numeric_limits<double>::max()) {
            lemon::Preflow<lemon::ListDigraph, lemon::ListDigraph::ArcMap<double>> preflow(mGraph, capacityMap, mSource, mSink);
            preflow.run();

            for (auto arc = lemon::ListDigraph::ArcIt(mGraph); arc != lemon::INVALID; ++arc) {
                auto arcSource = mGraph.source(arc);
                auto arcTarget = mGraph.target(arc);

                if (arcSource == mSource || arcTarget == mSink) {
                    continue;
                }

                auto sourceCell = mNode2Cell[arcSource];
                auto targetCell = mNode2Cell[arcTarget];


                if (preflow.minCut(arcSource) != preflow.minCut(arcTarget)) {
                    auto sourceCellLocation = mDesign.placement().cellLocation(sourceCell);
                    auto sourceCellBox = mDesign.placementMapping().geometry(sourceCell)[0];
                    auto targetCellLocation = mDesign.placement().cellLocation(targetCell);
                    auto targetCellBox = mDesign.placementMapping().geometry(targetCell)[0];

//                    std::cout << "changing edge from node " << mGraph.id(arcSource) << " to " << mGraph.id(arcTarget) << std::endl;
                    mGraph.erase(arc);
                    if (orthogonalComparator(sourceCellLocation, targetCellLocation)) {
                        orthogonalGraph.addEdge(sourceCell, targetCell);
                    } else {
                        orthogonalGraph.addEdge(targetCell, sourceCell);
                    }
                }
            }

            calculateSlacks(units::unit_cast<double>(min), units::unit_cast<double>(max));
            orthogonalGraph.calculateSlacks(units::unit_cast<double>(orthogonalMin), units::unit_cast<double>(orthogonalMax));
        }

    }

    void removeTransitiveEdges() {
        util::transitiveClosure(mGraph);
        util::transitiveReduction(mGraph);
    }

    void exportGraph(std::string fileName) {
        std::ofstream graphFile;
        graphFile.open (fileName);

        graphFile << "digraph G {" << std::endl;

        for (auto arc = lemon::ListDigraph::ArcIt(mGraph); arc != lemon::INVALID; ++arc) {
            auto source = mGraph.source(arc);
            auto target = mGraph.target(arc);

            graphFile << "\"" << mGraph.id(source) << " " << mMinimumLocations[source] << ", " << mMaximumLocations[source] << "\""
                      << " -> " << "\"" << mGraph.id(target) << " " << mMinimumLocations[target] << ", " << mMaximumLocations[target] << "\""
                            << " [label=" << mArcCosts[arc] << "];" << std::endl;
        }

        graphFile << "}" << std::endl;
    }

    void exportGraphToSvg(std::string fileName, geometry::Box area) {
        std::ofstream svgFile;
        svgFile.open (fileName);

        boost::geometry::svg_mapper<ophidian::geometry::Point> mapper(svgFile, 400, 400);

        mapper.add(area);
        mapper.map(area, "fill-opacity:0.5;fill:none;stroke:rgb(56,48,47);stroke-width:1", 5);

        for (auto node = lemon::ListDigraph::NodeIt(mGraph); node != lemon::INVALID; ++node) {
            if (node == mSource || node == mSink) {
                continue;
            }

            auto cell = mNode2Cell[node];

            auto cellLocation = mDesign.placement().cellLocation(cell).toPoint();
            auto cellBox = mDesign.placementMapping().geometry(cell)[0];
            auto cellWidth = cellBox.max_corner().x() - cellBox.min_corner().x();
            auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();

            mapper.add(cellBox);
            mapper.map(cellBox, "fill-opacity:0.5;fill:rgb(0,0,150);stroke:rgb(0,0,200);stroke-width:2", 5);

            geometry::Point textLocation(cellLocation.x(), cellLocation.y() + cellHeight / 2);
            mapper.text(textLocation, boost::lexical_cast<std::string>(mGraph.id(node)), "font-size:3px;fill-opacity:1.0;fill:rgb(56,48,47);stroke:none;stroke-width:1");
        }

        svgFile.close();
    }

protected:
    lemon::ListDigraph mGraph;

    lemon::ListDigraph::Node mSource;
    lemon::ListDigraph::Node mSink;

private:
    design::Design & mDesign;

    entity_system::Property<circuit::Cell, lemon::ListDigraph::Node> mCell2Node;
    lemon::ListDigraph::NodeMap<circuit::Cell> mNode2Cell;

    lemon::ListDigraph::ArcMap<double> mArcCosts;

    lemon::ListDigraph::NodeMap<double> mMinimumLocations;
    lemon::ListDigraph::NodeMap<double> mMaximumLocations;
};
}
}


#endif // CONSTRAINTGRAPH_H
