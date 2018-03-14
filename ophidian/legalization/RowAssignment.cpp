#include "RowAssignment.h"

namespace ophidian {
namespace legalization {
RowAssignment::RowAssignment(design::Design &design)
    : mDesign(design), mSubrows(design.netlist(), design.floorplan(), design.placement(), design.placementMapping()),
      mCircuitCellsSlices(mDesign.netlist().makeProperty<std::vector<CellSlice>>(circuit::Cell())),
      mSlice2Cell(mCellSlices), mSliceAlignment(mCellSlices)
{

}

void RowAssignment::assignCellsToRows()
{
    FenceRegionIsolation fenceRegionIsolation(mDesign);

    unsigned fenceIndex = 0;
    for(auto fence : mDesign.fences().range())
    {
        std::vector<circuit::Cell> cells (mDesign.fences().members(fence).begin(), mDesign.fences().members(fence).end());
        assignCellsToRows(mDesign.fences().area(fence), cells);
        fenceIndex++;
    }

    fenceRegionIsolation.isolateAllFenceCells();

    std::vector<circuit::Cell> cells;
    cells.reserve(mDesign.netlist().size(circuit::Cell()));
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
    {
        if(!mDesign.placement().cellHasFence(*cellIt))
        {
            cells.push_back(*cellIt);
        }
    }
    cells.shrink_to_fit();
    geometry::Box chipArea(mDesign.floorplan().chipOrigin().toPoint(), mDesign.floorplan().chipUpperRightCorner().toPoint());
    util::MultiBox legalizationArea({chipArea});
    assignCellsToRows(legalizationArea, cells);

    fenceRegionIsolation.restoreAllFenceCells();
}

void RowAssignment::addAssignmentVariables(CellSlice cellSlice, entity_system::Property<Subrow, GRBLinExpr> & capacityConstraints, entity_system::Property<CellSlice, std::vector<AssignmentPair>> & assignmentVariables, std::string cellName, util::Location cellLocation, GRBModel & model, double cellWidth, GRBLinExpr & objectiveFunction)
{
    assignmentVariables[cellSlice].reserve(mSubrows.rowCount());

    GRBLinExpr assignmentConstraintExpression;
    unsigned subrowIndex = 0;
    for (auto subrow : mSubrows.range(Subrow())) {
        if (sameAlignment(cellSlice, subrow)) {
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
    }

    model.addConstr(assignmentConstraintExpression == 1, cellName + "_assignmentConstraint");
}

bool RowAssignment::sameAlignment(CellSlice cellSlice, Subrow subrow)
{
    float rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    auto cellAlignment = mSliceAlignment[cellSlice];

    auto subrowOrigin = mSubrows.origin(subrow);
    int rowIndex = subrowOrigin.toPoint().y() / rowHeight;
    placement::RowAlignment rowAlignment = (rowIndex % 2 == 0) ? placement::RowAlignment::EVEN : placement::RowAlignment::ODD;

    return (cellAlignment == placement::RowAlignment::NA) || (cellAlignment == rowAlignment);
}

void RowAssignment::createAndSolveGurobiModel(std::vector<circuit::Cell> &cells) {
    mCellSlices.clear();
    sliceCells(cells);

    float rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    GRBLinExpr objectiveFunction;
    entity_system::Property<CellSlice, std::vector<AssignmentPair>> assignmentVariables(mCellSlices);
    entity_system::Property<Subrow, GRBLinExpr> capacityConstraints(mSubrows.makeProperty<GRBLinExpr>(Subrow()));

    for (auto cell : cells) {
        if (!mDesign.placement().isFixed(cell)) {
            auto cellName = mDesign.netlist().name(cell);

            auto cellLocation = mDesign.placement().cellLocation(cell);
            auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
            auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();

            auto cellSlices = mCircuitCellsSlices[cell];
            if (cellSlices.size() == 1) {
                auto cellSlice = cellSlices[0];
                addAssignmentVariables(cellSlice, capacityConstraints, assignmentVariables, cellName, cellLocation, model, cellWidth, objectiveFunction);
            } else {
                unsigned sliceIndex = 0;
                for (auto cellSlice : cellSlices) {
                    std::string sliceName = cellName + "_slice" + boost::lexical_cast<std::string>(sliceIndex);
                    util::Location sliceLocation(cellLocation.toPoint().x(), cellLocation.toPoint().y() + rowHeight * sliceIndex);
                    addAssignmentVariables(cellSlice, capacityConstraints, assignmentVariables, sliceName, sliceLocation, model, cellWidth, objectiveFunction);
                    sliceIndex++;
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

//    model.write("row_assignment.lp");

    for (auto cellSlice : mCellSlices) {
        auto circuitCell = mSlice2Cell[cellSlice];
        if (mCircuitCellsSlices[circuitCell][0] == cellSlice) {
            Subrow assignedSubrow;
            auto assignments = assignmentVariables[cellSlice];
            for (auto assignment : assignments) {
                double assignmentValue = assignment.first.get(GRB_DoubleAttr_X);
                if (assignmentValue > 0.0) {
                    assignedSubrow = assignment.second;
                    break;
                }
            }

            auto assignedSubrowOrigin = mSubrows.origin(assignedSubrow);
            util::Location finalLocation(mDesign.placement().cellLocation(circuitCell).x(), assignedSubrowOrigin.y());
            mDesign.placement().placeCell(circuitCell, finalLocation);
        }
    }
}

void RowAssignment::assignCellsToRows(util::MultiBox area, std::vector<circuit::Cell> &cells)
{
    if (cells.size() > 10000) {
        BinDecomposition binDecomposition(mDesign);
        binDecomposition.decomposeCircuitInBins(area, cells, 50);

        for (auto bin : binDecomposition.range(Bin())) {
            auto binBox = binDecomposition.box(bin);
            ophidian::util::MultiBox binArea({binBox});

            auto & binCells = binDecomposition.cells(bin);

            mSubrows.createSubrows(cells, binArea);

            createAndSolveGurobiModel(binCells);
        }
    } else {
        mSubrows.createSubrows(cells, area);
        createAndSolveGurobiModel(cells);
    }

}

void RowAssignment::sliceCells(std::vector<circuit::Cell> & cells)
{
    float rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    for (auto cell : cells) {
        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellHeigth = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        int numberOfSlices = cellHeigth/rowHeight;
        for (unsigned sliceIndex = 0; sliceIndex < numberOfSlices; sliceIndex++) {
            auto cellSlice = mCellSlices.add();
            mCircuitCellsSlices[cell].push_back(cellSlice);
            mSlice2Cell[cellSlice] = cell;
            if (sliceIndex == 0) {
                auto cellAlignment = mDesign.library().cellAlignment(mDesign.libraryMapping().cellStdCell(cell));
                mSliceAlignment[cellSlice] = cellAlignment;
            } else {
                mSliceAlignment[cellSlice] = placement::RowAlignment::NA;
            }
        }
    }
}
}
}

