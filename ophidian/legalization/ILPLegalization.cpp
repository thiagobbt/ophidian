#include "ILPLegalization.h"

namespace ophidian {
namespace legalization {
ILPLegalization::ILPLegalization(const circuit::Netlist &netlist, const floorplan::Floorplan &floorplan, placement::Placement &placement, const placement::PlacementMapping &placementMapping)
    : netlist_(netlist), floorplan_(floorplan), placement_(placement), placementMapping_(placementMapping) {

}

void ILPLegalization::legalizePlacement()
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

    GRBQuadExpr objective = 0.0;

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        auto cell = *cellIt;

        auto cellInitialLocation = placement_.cellLocation(cell);
        auto cellGeometry = placementMapping_.geometry(cell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        auto cellName = netlist_.name(cell);
        GRBVar x = model.addVar(units::unit_cast<double>(chipOrigin.x()), chipRightX - cellWidth,
                                0.0, GRB_CONTINUOUS, cellName + "_x");
        xVariables[cell] = x;
        GRBVar y = model.addVar(units::unit_cast<double>(chipOrigin.y()), chipTopY - cellHeight,
                                0.0, GRB_CONTINUOUS, cellName + "_y");
        yVariables[cell] = y;

        double initialX = cellInitialLocation.toPoint().x();
        double initialY = cellInitialLocation.toPoint().y();
        objective += (initialX - x)*(initialX - x);
        objective += (initialY - y)*(initialY - y);
    }

    for (auto cellIt1 = netlist_.begin(circuit::Cell()); cellIt1 != netlist_.end(circuit::Cell()); ++cellIt1) {
        for (auto cellIt2 = netlist_.begin(circuit::Cell()); cellIt2 != cellIt1; ++cellIt2) {
            if (cellIt1 != cellIt2) {
                auto cell1 = *cellIt1;
                auto cell2 = *cellIt2;

                auto cell1Name = netlist_.name(cell1);
                auto cell2Name = netlist_.name(cell2);

                auto cell1X = xVariables[cell1];
                auto cell1Y = yVariables[cell1];
                auto cell2X = xVariables[cell2];
                auto cell2Y = yVariables[cell2];

                auto cell1Location = placement_.cellLocation(cell1).toPoint();
                auto cell1Geometry = placementMapping_.geometry(cell1);
                auto cell1Width = cell1Geometry[0].max_corner().x() - cell1Geometry[0].min_corner().x();
                auto cell1Height = cell1Geometry[0].max_corner().y() - cell1Geometry[0].min_corner().y();

                auto cell2Location = placement_.cellLocation(cell2).toPoint();
                auto cell2Geometry = placementMapping_.geometry(cell2);
                auto cell2Width = cell2Geometry[0].max_corner().x() - cell2Geometry[0].min_corner().x();
                auto cell2Height = cell2Geometry[0].max_corner().y() - cell2Geometry[0].min_corner().y();

                double xDistance = (cell1Location.x() <= cell2Location.x()) ? cell2Location.x() - (cell1Location.x() + cell1Width) : cell1Location.x() - (cell2Location.x() + cell2Width);
                double yDistance = (cell1Location.y() <= cell2Location.y()) ? cell2Location.y() - (cell1Location.y() + cell1Height) : cell1Location.y() - (cell2Location.y() + cell2Height);

                if (xDistance >= 0 && yDistance >= 0) {
                    if (xDistance > yDistance) {
                        addOverlapConstraint(model, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                    } else {
                        addOverlapConstraint(model, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                    }
                } else if (xDistance >= 0) {
                    addOverlapConstraint(model, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                } else if (yDistance >= 0) {
                    addOverlapConstraint(model, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                } else {
                    if (xDistance > yDistance) {
                        addOverlapConstraint(model, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                    } else {
                        addOverlapConstraint(model, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                    }
                }
            }
        }
    }

    model.setObjective(objective, GRB_MINIMIZE);

    model.optimize();

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        auto cell = *cellIt;

        auto cellX = xVariables[cell];
        auto cellY = yVariables[cell];

        auto cellXLocation = std::round(cellX.get(GRB_DoubleAttr_X) / siteWidth) * siteWidth;
        auto cellYLocation = std::round(cellY.get(GRB_DoubleAttr_X) / rowHeight) * rowHeight;

        util::Location legalLocation(cellXLocation, cellYLocation);
        placement_.placeCell(cell, legalLocation);
    }
}

void ILPLegalization::legalize(const std::vector<circuit::Cell> &cells, const geometry::Box &legalizationArea)
{
    auto siteWidth = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).toPoint().x();
    auto rowHeight = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).toPoint().y();

    auto chipOrigin = legalizationArea.min_corner();
    auto upperRightCorner = legalizationArea.max_corner();
    double chipRightX = std::floor(upperRightCorner.x() / siteWidth) * siteWidth;
    double chipTopY = std::floor(upperRightCorner.y() / rowHeight) * rowHeight;

    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    entity_system::Property<circuit::Cell, GRBVar> xVariables = netlist_.makeProperty<GRBVar>(circuit::Cell());
    entity_system::Property<circuit::Cell, GRBVar> yVariables = netlist_.makeProperty<GRBVar>(circuit::Cell());

    GRBQuadExpr objective = 0.0;

    for (auto cell : cells) {
        auto cellInitialLocation = placement_.cellLocation(cell);
        auto cellGeometry = placementMapping_.geometry(cell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        auto cellName = netlist_.name(cell);
        GRBVar x = model.addVar(chipOrigin.x(), chipRightX - cellWidth,
                                0.0, GRB_CONTINUOUS, cellName + "_x");
        xVariables[cell] = x;
        GRBVar y = model.addVar(chipOrigin.y(), chipTopY - cellHeight,
                                0.0, GRB_CONTINUOUS, cellName + "_y");
        yVariables[cell] = y;

        double initialX = cellInitialLocation.toPoint().x();
        double initialY = cellInitialLocation.toPoint().y();
        objective += (initialX - x)*(initialX - x);
        objective += (initialY - y)*(initialY - y);
    }

    for (auto cell1 : cells) {
        for (auto cell2 : cells) {
            if (cell1 != cell2) {
                auto cell1Name = netlist_.name(cell1);
                auto cell2Name = netlist_.name(cell2);

                auto cell1X = xVariables[cell1];
                auto cell1Y = yVariables[cell1];
                auto cell2X = xVariables[cell2];
                auto cell2Y = yVariables[cell2];

                auto cell1Location = placement_.cellLocation(cell1).toPoint();
                auto cell1Geometry = placementMapping_.geometry(cell1);
                auto cell1Width = cell1Geometry[0].max_corner().x() - cell1Geometry[0].min_corner().x();
                auto cell1Height = cell1Geometry[0].max_corner().y() - cell1Geometry[0].min_corner().y();

                auto cell2Location = placement_.cellLocation(cell2).toPoint();
                auto cell2Geometry = placementMapping_.geometry(cell2);
                auto cell2Width = cell2Geometry[0].max_corner().x() - cell2Geometry[0].min_corner().x();
                auto cell2Height = cell2Geometry[0].max_corner().y() - cell2Geometry[0].min_corner().y();

                double xDistance = (cell1Location.x() <= cell2Location.x()) ? cell2Location.x() - (cell1Location.x() + cell1Width) : cell1Location.x() - (cell2Location.x() + cell2Width);
                double yDistance = (cell1Location.y() <= cell2Location.y()) ? cell2Location.y() - (cell1Location.y() + cell1Height) : cell1Location.y() - (cell2Location.y() + cell2Height);

                if (xDistance >= 0 && yDistance >= 0) {
                    if (xDistance > yDistance) {
                        addOverlapConstraint(model, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                    } else {
                        addOverlapConstraint(model, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                    }
                } else if (xDistance >= 0) {
                    addOverlapConstraint(model, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                } else if (yDistance >= 0) {
                    addOverlapConstraint(model, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                } else {
                    if (xDistance > yDistance) {
                        addOverlapConstraint(model, cell1Name, cell2Name, cell1X, cell2X, cell1Location.x(), cell2Location.x(), cell1Width, cell2Width);
                    } else {
                        addOverlapConstraint(model, cell1Name, cell2Name, cell1Y, cell2Y, cell1Location.y(), cell2Location.y(), cell1Height, cell2Height);
                    }
                }
            }
        }
    }

    model.setObjective(objective, GRB_MINIMIZE);

    model.optimize();

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        auto cell = *cellIt;

        auto cellX = xVariables[cell];
        auto cellY = yVariables[cell];

        auto cellXLocation = std::round(cellX.get(GRB_DoubleAttr_X) / siteWidth) * siteWidth;
        auto cellYLocation = std::round(cellY.get(GRB_DoubleAttr_X) / rowHeight) * rowHeight;

        util::Location legalLocation(cellXLocation, cellYLocation);
        placement_.placeCell(cell, legalLocation);
    }
}

void ILPLegalization::addOverlapConstraint(GRBModel & model, std::string cell1Name, std::string cell2Name, GRBVar cell1Var, GRBVar cell2Var, double cell1Coordinate, double cell2Coordinate, double cell1Dimension, double cell2Dimension)
{
    if (cell1Coordinate <= cell2Coordinate) {
        model.addConstr(cell1Var + cell1Dimension <= cell2Var, cell1Name + "_" + cell2Name + "_overlap_constraint");
    } else {
        model.addConstr(cell2Var + cell2Dimension <= cell1Var, cell1Name + "_" + cell2Name + "_overlap_constraint");
    }
}

}
}
