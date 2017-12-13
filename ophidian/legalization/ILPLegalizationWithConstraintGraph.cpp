#include "ILPLegalizationWithConstraintGraph.h"

namespace ophidian {
namespace legalization {
ILPLegalizationWithConstraintGraph::ILPLegalizationWithConstraintGraph(design::Design &design)
    : mDesign(design), mHorizontalConstraintGraph(mDesign), mVerticalConstraintGraph(mDesign) {

}

void ILPLegalizationWithConstraintGraph::legalize(const std::vector<circuit::Cell> &cells, const geometry::Box & legalizationArea)
{
    auto siteWidth = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().x();
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    auto origin = legalizationArea.min_corner();
    auto upperRightCorner = legalizationArea.max_corner();
    double rightX = std::floor(upperRightCorner.x() / siteWidth) * siteWidth;
    double topY = std::floor(upperRightCorner.y() / rowHeight) * rowHeight;

    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    entity_system::Property<circuit::Cell, GRBVar> xVariables = mDesign.netlist().makeProperty<GRBVar>(circuit::Cell());
    entity_system::Property<circuit::Cell, GRBVar> yVariables = mDesign.netlist().makeProperty<GRBVar>(circuit::Cell());

    GRBQuadExpr objective = 0.0;

    for (auto cell : cells) {
        auto cellInitialLocation = mDesign.placement().cellLocation(cell);
        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        auto cellName = mDesign.netlist().name(cell);
        GRBVar x = model.addVar(origin.x(), rightX - cellWidth,
                                0.0, GRB_CONTINUOUS, cellName + "_x");
        xVariables[cell] = x;
        GRBVar y = model.addVar(origin.y(), topY - cellHeight,
                                0.0, GRB_CONTINUOUS, cellName + "_y");
        yVariables[cell] = y;

        double initialX = cellInitialLocation.toPoint().x();
        double initialY = cellInitialLocation.toPoint().y();
        objective += (initialX - x)*(initialX - x);
        objective += (initialY - y)*(initialY - y);
    }

    mHorizontalConstraintGraph.buildConstraintGraph(cells, mDesign.floorplan().chipOrigin().x(), mDesign.floorplan().chipUpperRightCorner().x());
    mVerticalConstraintGraph.buildConstraintGraph(cells, mDesign.floorplan().chipOrigin().y(), mDesign.floorplan().chipUpperRightCorner().y());
    mHorizontalConstraintGraph.removeTransitiveEdges();
    mVerticalConstraintGraph.removeTransitiveEdges();

    for (auto cell1 : cells) {
        for (auto cell2 : cells) {
            if (cell1 != cell2) {
                auto cell1Geometry = mDesign.placementMapping().geometry(cell1)[0];
                auto cell1Name = mDesign.netlist().name(cell1);
                auto cell2Name = mDesign.netlist().name(cell2);

                if (mHorizontalConstraintGraph.hasEdge(cell1, cell2)) {
                    auto cell1Var = xVariables[cell1];
                    auto cell1Width = cell1Geometry.max_corner().x() - cell1Geometry.min_corner().x();
                    auto cell2Var = xVariables[cell2];
                    model.addConstr(cell1Var + cell1Width <= cell2Var, cell1Name + "_" + cell2Name + "_horizontal_overlap_constraint");
                } else if (mVerticalConstraintGraph.hasEdge(cell1, cell2)) {
                    auto cell1Var = yVariables[cell1];
                    auto cell1Height = cell1Geometry.max_corner().y() - cell1Geometry.min_corner().y();
                    auto cell2Var = yVariables[cell2];
                    model.addConstr(cell1Var + cell1Height <= cell2Var, cell1Name + "_" + cell2Name + "_vertical_overlap_constraint");
                }
            }
        }
    }

    model.setObjective(objective, GRB_MINIMIZE);

    model.write("ilp_legalization.lp");

    model.optimize();

    if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
        for (auto cell : cells) {
            auto cellX = xVariables[cell];
            auto cellY = yVariables[cell];

            auto cellXLocation = std::round(cellX.get(GRB_DoubleAttr_X) / siteWidth) * siteWidth;
            auto cellYLocation = std::round(cellY.get(GRB_DoubleAttr_X) / rowHeight) * rowHeight;

            util::Location legalLocation(cellXLocation, cellYLocation);
            mDesign.placement().placeCell(cell, legalLocation);
        }
    }
}

void ILPLegalizationWithConstraintGraph::writeGraphFile(const std::vector<circuit::Cell> & cells)
{
    std::ofstream horizontalGraphFile;
    horizontalGraphFile.open ("horizontal_graph.gv");
    horizontalGraphFile << "digraph G {" << std::endl;

    std::ofstream verticalGraphFile;
    verticalGraphFile.open ("vertical_graph.gv");
    verticalGraphFile << "digraph G {" << std::endl;

    unsigned cell1Index = 0;
    for (auto cell1 : cells) {
        unsigned cell2Index = 0;
        for (auto cell2 : cells) {
            auto cell1Geometry = mDesign.placementMapping().geometry(cell1)[0];
            auto cell1Width = cell1Geometry.max_corner().x() - cell1Geometry.min_corner().x();
            auto cell1Height = cell1Geometry.max_corner().y() - cell1Geometry.min_corner().y();

            auto cell2Geometry = mDesign.placementMapping().geometry(cell2)[0];
            auto cell2Width = cell2Geometry.max_corner().x() - cell2Geometry.min_corner().x();
            auto cell2Height = cell2Geometry.max_corner().y() - cell2Geometry.min_corner().y();

            if (mHorizontalConstraintGraph.hasEdge(cell1, cell2)) {
//                horizontalGraphFile << cell1Index << " -> " << cell2Index << std::endl;
                horizontalGraphFile << cell1Index << " -> " << cell2Index << " [label=" << cell1Width << "]" << std::endl;
            }
            if (mVerticalConstraintGraph.hasEdge(cell1, cell2)) {
//                verticalGraphFile << cell1Index << " -> " << cell2Index << std::endl;
                verticalGraphFile << cell1Index << " -> " << cell2Index << " [label=" << cell1Height << "]" << std::endl;
            }
            cell2Index++;
        }
        cell1Index++;
    }

    horizontalGraphFile << "}" << std::endl;
    verticalGraphFile << "}" << std::endl;
}
}
}
