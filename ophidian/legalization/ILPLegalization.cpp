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
    auto siteWidth = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).x();
    auto rowHeight = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).y();

    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    entity_system::Property<circuit::Cell, GRBVar> xVariables = netlist_.makeProperty<GRBVar>(circuit::Cell());
    entity_system::Property<circuit::Cell, GRBVar> yVariables = netlist_.makeProperty<GRBVar>(circuit::Cell());

    GRBLinExpr objective = 0.0;
//    GRBQuadExpr objective = 0.0;

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        auto cellInitialLocation = placement_.cellLocation(*cellIt);
        auto cellName = netlist_.name(*cellIt);
        GRBVar x = model.addVar(units::unit_cast<double>(chipOrigin.x()), units::unit_cast<double>(chipUpperRightCorner.x()),
                                0.0, GRB_INTEGER, cellName + "_x");
        xVariables[*cellIt] = x;
        GRBVar y = model.addVar(units::unit_cast<double>(chipOrigin.y()), units::unit_cast<double>(chipUpperRightCorner.y()),
                                0.0, GRB_INTEGER, cellName + "_y");
        yVariables[*cellIt] = y;
        GRBVar Dx = model.addVar(0.0, units::unit_cast<double>(chipUpperRightCorner.x()),
                                 0.0, GRB_CONTINUOUS, cellName + "_Dx");
        GRBVar Dy = model.addVar(0.0, units::unit_cast<double>(chipUpperRightCorner.y()),
                                 0.0, GRB_CONTINUOUS, cellName + "_Dy");

        model.addConstr(units::unit_cast<double>(cellInitialLocation.x()) - x - Dx <= 0, cellName + "_Dx_constraint1");
        model.addConstr(x - units::unit_cast<double>(cellInitialLocation.x()) - Dx <= 0, cellName + "_Dx_constraint2");
        model.addConstr(units::unit_cast<double>(cellInitialLocation.y()) - y - Dy <= 0, cellName + "_Dy_constraint1");
        model.addConstr(y - units::unit_cast<double>(cellInitialLocation.y()) - Dy <= 0, cellName + "_Dy_constraint2");

        GRBVar n = model.addVar(0, units::unit_cast<double>(chipUpperRightCorner.x() / siteWidth), 0.0, GRB_INTEGER, cellName + "_site_alignment_variable");
        GRBVar m = model.addVar(0, units::unit_cast<double>(chipUpperRightCorner.y() / rowHeight), 0.0, GRB_INTEGER, cellName + "_row_alignment_variable");

        model.addConstr(x - n*units::unit_cast<double>(siteWidth) == 0, cellName + "_site_alignment_constraint");
        model.addConstr(y - m*units::unit_cast<double>(rowHeight) == 0, cellName + "_row_alignment_constraint");

//        double initialX = units::unit_cast<double>(cellInitialLocation.x());
//        double initialY = units::unit_cast<double>(cellInitialLocation.y());
//        objective += (initialX - x)*(initialX - x);
//        objective += (initialY - y)*(initialY - y);

         objective += Dx + Dy;
    }

    for (auto cellIt1 = netlist_.begin(circuit::Cell()); cellIt1 != netlist_.end(circuit::Cell()); ++cellIt1) {
        for (auto cellIt2 = netlist_.begin(circuit::Cell()); cellIt2 != netlist_.end(circuit::Cell()); ++cellIt2) {
            if (cellIt1 != cellIt2) {
                auto cell1Name = netlist_.name(*cellIt1);
                auto cell2Name = netlist_.name(*cellIt2);

                auto cell1X = xVariables[*cellIt1];
                auto cell1Y = yVariables[*cellIt1];
                auto cell2X = xVariables[*cellIt2];
                auto cell2Y = yVariables[*cellIt2];

                auto cell1Geometry = placementMapping_.geometry(*cellIt1);
                auto cell1Width = cell1Geometry[0].max_corner().x() - cell1Geometry[0].min_corner().x();
                auto cell1Height = cell1Geometry[0].max_corner().y() - cell1Geometry[0].min_corner().y();
                auto cell2Geometry = placementMapping_.geometry(*cellIt2);
                auto cell2Width = cell2Geometry[0].max_corner().x() - cell2Geometry[0].min_corner().x();
                auto cell2Height = cell2Geometry[0].max_corner().y() - cell2Geometry[0].min_corner().y();

                GRBVar left = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, cell1Name + "_" + cell2Name + "_left");
                GRBVar right = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, cell1Name + "_" + cell2Name + "_right");
                GRBVar bottom = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, cell1Name + "_" + cell2Name + "_bottom");
                GRBVar top = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, cell1Name + "_" + cell2Name + "_top");

    //            GRBVar y = model.addVar(-units::unit_cast<double>(chipUpperRightCorner.x()),
    //                                     units::unit_cast<double>(chipUpperRightCorner.x()),
    //                                     0.0, GRB_INTEGER, cell1Name + "_" + cell2Name + "_y");


    //            model.addConstr(y - (cell1X + cell1Width - cell2X) <= 0, cell1Name + "_" + cell2Name + "_y_left_constraint1");
    //            model.addConstr(y - (cell1X + cell1Width - cell2X - 2*chipRightX*left) >= 0, cell1Name + "_" + cell2Name + "_y_left_constraint2");
    //            model.addConstr(y - (cell2X + cell2Width - cell1X) <= 0, cell1Name + "_" + cell2Name + "_y_right_constraint1");
    //            model.addConstr(y - (cell2X + cell2Width - cell1X - 2*chipRightX*right) >= 0, cell1Name + "_" + cell2Name + "_y_right_constraint2");
    //            model.addConstr(y - (cell1Y + cell1Height - cell2Y) <= 0, cell1Name + "_" + cell2Name + "_y_bottom_constraint1");
    //            model.addConstr(y - (cell1Y + cell1Height - cell2Y - 2*chipTopY*bottom) >= 0, cell1Name + "_" + cell2Name + "_y_bottom_constraint2");
    //            model.addConstr(y - (cell2Y + cell2Height - cell1Y) <= 0, cell1Name + "_" + cell2Name + "_y_top_constraint1");
    //            model.addConstr(y - (cell2Y + cell2Height - cell1Y - 2*chipTopY*top) >= 0, cell1Name + "_" + cell2Name + "_y_top_constraint2");
    //            model.addConstr(y <= 0, cell1Name + "_" + cell2Name + "_overlap_constraint1");

                model.addConstr(cell1X + cell1Width - cell2X - (1-left)*chipRightX <= 0, cell1Name + "_" + cell2Name + "_y_left_constraint");
                model.addConstr(cell2X + cell2Width - cell1X - (1-right)*chipRightX <= 0, cell1Name + "_" + cell2Name + "_y_right_constraint");
                model.addConstr(cell1Y + cell1Height - cell2Y - (1-bottom)*chipTopY <= 0, cell1Name + "_" + cell2Name + "_y_bottom_constraint");
                model.addConstr(cell2Y + cell2Height - cell1Y - (1-top)*chipTopY <= 0, cell1Name + "_" + cell2Name + "_y_top_constraint");

                model.addConstr(left + right <= 1, cell1Name + "_" + cell2Name + "_horizontal_overlap_constraint");
                model.addConstr(bottom + top <= 1, cell1Name + "_" + cell2Name + "_vertical_overlap_constraint");
                model.addConstr(left + right + bottom + top >= 1, cell1Name + "_" + cell2Name + "_total_overlap_constraint");

    //            model.addConstr(left*(cell1X + cell1Width - cell2X) +
    //                            right*(cell2X + cell2Width - cell1X) +
    //                            bottom*(cell1Y + cell1Height - cell2Y) +
    //                            top*(cell2Y + cell2Height - cell1Y) <= 0, cell1Name + "_" + cell2Name + "_overlap_constraint");
            }
        }
    }

    model.setObjective(objective, GRB_MINIMIZE);

    model.optimize();

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        auto cellX = xVariables[*cellIt];
        auto cellY = yVariables[*cellIt];

        util::Location legalLocation(cellX.get(GRB_DoubleAttr_X), cellY.get(GRB_DoubleAttr_X));
        placement_.placeCell(*cellIt, legalLocation);
    }
}
}
}
