#include "RowAssignment.h"

namespace ophidian {
namespace legalization {
RowAssignment::RowAssignment(design::Design &design)
    : mDesign(design), mSubrows(design.netlist(), design.floorplan(), design.placement(), design.placementMapping()),
        mCircuitCellsSlices(mDesign.netlist().makeProperty<std::vector<CellSlice>>(circuit::Cell())),
        mSlice2Cell(mCellSlices)
{

}

void RowAssignment::addAssignmentVariables(CellSlice cellSlice, entity_system::Property<Subrow, GRBLinExpr> & capacityConstraints, entity_system::Property<CellSlice, std::vector<AssignmentPair>> & assignmentVariables, std::string cellName, util::Location cellLocation, GRBModel & model, double cellWidth, GRBLinExpr & objectiveFunction)
{
    assignmentVariables[cellSlice].reserve(mSubrows.rowCount());

    GRBLinExpr assignmentConstraintExpression;
    unsigned subrowIndex = 0;
    for (auto subrow : mSubrows.range(Subrow())) {
        std::string variableName = cellName + "_" + boost::lexical_cast<std::string>(subrowIndex++) + "_assignment";
        GRBVar assignmentVariable = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, variableName);
        assignmentConstraintExpression += assignmentVariable;

        assignmentVariables[cellSlice].push_back(std::make_pair(assignmentVariable, subrow));

        GRBLinExpr & capacityConstraint = capacityConstraints[subrow];
        capacityConstraint += cellWidth*assignmentVariable;

        auto subrowOrigin = mSubrows.origin(subrow);
        auto subrowUpperCorner = mSubrows.upperCorner(subrow);

        double displacement = std::abs(cellLocation.toPoint().y() - subrowOrigin.toPoint().y());
        if (cellLocation.x() < subrowOrigin.x()) {
            displacement = displacement + std::abs(cellLocation.toPoint().x() - subrowOrigin.toPoint().x());
        } else if (cellLocation.toPoint().x() + cellWidth > subrowUpperCorner.toPoint().x()) {
            displacement = displacement + std::abs(cellLocation.toPoint().x() + cellWidth - subrowUpperCorner.toPoint().x());
        }

        objectiveFunction += displacement * assignmentVariable;
    }

    model.addConstr(assignmentConstraintExpression == 1, cellName + "_assignmentConstraint");
}

void RowAssignment::assignCellsToRows(util::MultiBox area)
{
    mSubrows.createSubrows(area);

    sliceCells();

    float rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    GRBLinExpr objectiveFunction;
    entity_system::Property<CellSlice, std::vector<AssignmentPair>> assignmentVariables(mCellSlices);
    entity_system::Property<Subrow, GRBLinExpr> capacityConstraints(mSubrows.makeProperty<GRBLinExpr>(Subrow()));

    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
        if (!mDesign.placement().isFixed(*cellIt)) {
            auto cellName = mDesign.netlist().name(*cellIt);

            auto cellLocation = mDesign.placement().cellLocation(*cellIt);
            auto cellGeometry = mDesign.placementMapping().geometry(*cellIt)[0];
            auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();

            auto cellSlices = mCircuitCellsSlices[*cellIt];
            if (cellSlices.size() == 1) {
                auto cellSlice = cellSlices[0];
                addAssignmentVariables(cellSlice, capacityConstraints, assignmentVariables, cellName, cellLocation, model, cellWidth, objectiveFunction);
            } else {
                unsigned sliceIndex = 0;
                for (auto cellSlice : cellSlices) {
                    std::string sliceName = cellName + "_slice" + boost::lexical_cast<std::string>(sliceIndex);
                    util::Location sliceLocation(cellLocation.toPoint().x(), cellLocation.toPoint().y() + rowHeight * sliceIndex);
                    addAssignmentVariables(cellSlice, capacityConstraints, assignmentVariables, sliceName, sliceLocation, model, cellWidth, objectiveFunction);
                }

                for (unsigned sliceIndex = 1; sliceIndex < cellSlices.size(); sliceIndex++) {
                    auto currentSlice = cellSlices[sliceIndex];
                    auto previousSlice = cellSlices[sliceIndex-1];

                    GRBLinExpr sliceDistanceConstraint = 0.0;
                    for (auto variable : assignmentVariables[currentSlice]) {
                        auto subrow = variable.second;
                        auto subrowOrigin = mSubrows.origin(subrow).toPoint();
                        sliceDistanceConstraint += subrowOrigin.y() * variable.first;
                    }
                    for (auto variable : assignmentVariables[previousSlice]) {
                        auto subrow = variable.second;
                        auto subrowOrigin = mSubrows.origin(subrow).toPoint();
                        sliceDistanceConstraint -= subrowOrigin.y() * variable.first;
                    }
                    model.addConstr(sliceDistanceConstraint == rowHeight, cellName + "_slice_constraint");
                }
            }

        }
    }

    unsigned subrowIndex = 0;
    for (auto subrow : mSubrows.range(Subrow())) {
        auto subrowCapacity = mSubrows.capacity(subrow);

        GRBLinExpr capacityConstraint = capacityConstraints[subrow];
        model.addConstr(capacityConstraint <= units::unit_cast<double>(subrowCapacity), boost::lexical_cast<std::string>(subrowIndex++) + "_capacity");
    }

    model.setObjective(objectiveFunction, GRB_MINIMIZE);
    model.optimize();

    for (auto cellSlice : mCellSlices) {
        auto circuitCell = mSlice2Cell[cellSlice];
        if (mCircuitCellsSlices[circuitCell][0] == cellSlice) {
            Subrow assignedSubrow;
            auto assignments = assignmentVariables[cellSlice];
            for (auto assignment : assignments) {
                double assignmentValue = assignment.first.get(GRB_DoubleAttr_X);
                if (assignmentValue == 1.0) {
                    assignedSubrow = assignment.second;
                    break;
                }
            }

            auto assignedSubrowOrigin = mSubrows.origin(assignedSubrow);
            mDesign.placement().placeCell(circuitCell, assignedSubrowOrigin);
        }
    }
}

void RowAssignment::sliceCells()
{
    float rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
        auto cellGeometry = mDesign.placementMapping().geometry(*cellIt)[0];
        auto cellHeigth = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        int numberOfSlices = cellHeigth/rowHeight;
        for (unsigned sliceIndex = 0; sliceIndex < numberOfSlices; sliceIndex++) {
            auto cellSlice = mCellSlices.add();
            mCircuitCellsSlices[*cellIt].push_back(cellSlice);
            mSlice2Cell[cellSlice] = *cellIt;
        }
    }
}
}
}
