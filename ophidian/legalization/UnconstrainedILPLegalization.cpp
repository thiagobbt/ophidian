#include "UnconstrainedILPLegalization.h"

namespace ophidian {
namespace legalization {
UnconstrainedILPLegalization::UnconstrainedILPLegalization(const circuit::Netlist &netlist, const floorplan::Floorplan &floorplan, placement::Placement &placement, const placement::PlacementMapping &placementMapping)
    : netlist_(netlist), floorplan_(floorplan), placement_(placement), placementMapping_(placementMapping) {

}

void UnconstrainedILPLegalization::legalizePlacement()
{
    auto chipOrigin = floorplan_.chipOrigin();
    auto chipUpperRightCorner = floorplan_.chipUpperRightCorner();
    double chipRightX = units::unit_cast<double>(chipUpperRightCorner.x());
    double chipTopY = units::unit_cast<double>(chipUpperRightCorner.y());
    auto siteWidth = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).toPoint().x();
    auto rowHeight = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).toPoint().y();

    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    entity_system::Property<circuit::Cell, GRBVar> xVariables = netlist_.makeProperty<GRBVar>(circuit::Cell());
    entity_system::Property<circuit::Cell, GRBVar> yVariables = netlist_.makeProperty<GRBVar>(circuit::Cell());

//    GRBLinExpr objective = 0.0;

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        auto cellInitialLocation = placement_.cellLocation(*cellIt);
        auto cellGeometry = placementMapping_.geometry(*cellIt)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        auto cellName = netlist_.name(*cellIt);
        GRBVar x = model.addVar(units::unit_cast<double>(chipOrigin.x()), chipRightX - cellWidth,
                                cellInitialLocation.toPoint().x(), GRB_CONTINUOUS, cellName + "_x");
        xVariables[*cellIt] = x;
        GRBVar y = model.addVar(units::unit_cast<double>(chipOrigin.y()), chipTopY - cellHeight,
                                cellInitialLocation.toPoint().y(), GRB_CONTINUOUS, cellName + "_y");
        yVariables[*cellIt] = y;

//        double initialX = cellInitialLocation.toPoint().x();
//        double initialY = cellInitialLocation.toPoint().y();
//        objective += (initialX - x)*(initialX - x);
//        objective += (initialY - y)*(initialY - y);
    }

    GRBVar objectiveVariable = model.addVar(0.0, std::max(chipRightX, chipTopY), 0.0, GRB_CONTINUOUS, "objective_variable");

    for (auto cellIt1 = netlist_.begin(circuit::Cell()); cellIt1 != netlist_.end(circuit::Cell()); ++cellIt1) {
        for (auto cellIt2 = netlist_.begin(circuit::Cell()); cellIt2 != cellIt1; ++cellIt2) {
            if (cellIt1 != cellIt2) {
                auto cell1Name = netlist_.name(*cellIt1);
                auto cell2Name = netlist_.name(*cellIt2);

                auto cell1X = xVariables[*cellIt1];
                auto cell1Y = yVariables[*cellIt1];
                auto cell2X = xVariables[*cellIt2];
                auto cell2Y = yVariables[*cellIt2];

                auto cell1Location = placement_.cellLocation(*cellIt1).toPoint();
                auto cell1Geometry = placementMapping_.geometry(*cellIt1);
                auto cell1Width = cell1Geometry[0].max_corner().x() - cell1Geometry[0].min_corner().x();
                auto cell1Height = cell1Geometry[0].max_corner().y() - cell1Geometry[0].min_corner().y();

                auto cell2Location = placement_.cellLocation(*cellIt2).toPoint();
                auto cell2Geometry = placementMapping_.geometry(*cellIt2);
                auto cell2Width = cell2Geometry[0].max_corner().x() - cell2Geometry[0].min_corner().x();
                auto cell2Height = cell2Geometry[0].max_corner().y() - cell2Geometry[0].min_corner().y();

                double xDistance = (cell1Location.x() <= cell2Location.x()) ? cell2Location.x() - (cell1Location.x() + cell1Width) : cell1Location.x() - (cell2Location.x() + cell2Width);
                double yDistance = (cell1Location.y() <= cell2Location.y()) ? cell2Location.y() - (cell1Location.y() + cell1Height) : cell1Location.y() - (cell2Location.y() + cell2Height);

                if (xDistance >= 0 && yDistance >= 0) {
                    if (xDistance > yDistance) {
                        addOverlapToObjective(model, objectiveVariable, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                    } else {
                        addOverlapToObjective(model, objectiveVariable, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                    }
                } else if (xDistance >= 0) {
                    addOverlapToObjective(model, objectiveVariable, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                } else if (yDistance >= 0) {
                    addOverlapToObjective(model, objectiveVariable, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                } else {
                    if (xDistance > yDistance) {
                        addOverlapToObjective(model, objectiveVariable, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                    } else {
                        addOverlapToObjective(model, objectiveVariable, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                    }
                }
            }
        }
    }

    GRBLinExpr objective = objectiveVariable;
    model.setObjective(objective, GRB_MAXIMIZE);

    model.optimize();

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        auto cellX = xVariables[*cellIt];
        auto cellY = yVariables[*cellIt];

        auto cellXLocation = std::round(cellX.get(GRB_DoubleAttr_X) / siteWidth) * siteWidth;
        auto cellYLocation = std::round(cellY.get(GRB_DoubleAttr_X) / rowHeight) * rowHeight;

        util::Location legalLocation(cellXLocation, cellYLocation);
        placement_.placeCell(*cellIt, legalLocation);
    }
}

void UnconstrainedILPLegalization::addOverlapToObjective(GRBModel & model, GRBVar & objectiveVariable, std::string cell1Name, std::string cell2Name, GRBVar cell1Var, GRBVar cell2Var, double cell1Coordinate, double cell2Coordinate, double cell1Dimension, double cell2Dimension)
{
    if (cell1Coordinate <= cell2Coordinate) {
//        objective += cell2Var - (cell1Var + cell1Dimension);
        model.addConstr(objectiveVariable <= cell2Var - (cell1Var + cell1Dimension), cell1Name + "_" + cell2Name + "_overlap_constraint");
    } else {
//        objective += cell1Var - (cell2Var + cell2Dimension);
        model.addConstr(objectiveVariable <= cell1Var - (cell2Var + cell2Dimension), cell1Name + "_" + cell2Name + "_overlap_constraint");
    }
}

}
}
