#include "CellShifting.h"

namespace ophidian {
namespace legalization {
CellShifting::CellShifting(design::Design & design)
    : mDesign(design), mSubrows(design.netlist(), design.floorplan(), design.placement(), design.placementMapping()),
      mCircuitCellsSlices(mDesign.netlist().makeProperty<std::vector<CellSlice>>(circuit::Cell())),
      mSlice2Cell(mCellSlices), mSliceAlignment(mCellSlices), mSliceNames(mCellSlices),
      mCellsInitialLocations(design.netlist().makeProperty<util::Location>(circuit::Cell()))
{
    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); cellIt++) {
        mCellsInitialLocations[*cellIt] = mDesign.placement().cellLocation(*cellIt);
    }
}

void CellShifting::shiftCellsInsideRows()
{
    FenceRegionIsolation fenceRegionIsolation(mDesign);

    for(auto fence : mDesign.fences().range())
    {
        std::vector<circuit::Cell> cells (mDesign.fences().members(fence).begin(), mDesign.fences().members(fence).end());
        shiftCellsInsideRows(mDesign.fences().area(fence), cells);
    }

    fenceRegionIsolation.isolateAllFenceCells();

    //posiciona circuito
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
    shiftCellsInsideRows(legalizationArea, cells);

    fenceRegionIsolation.restoreAllFenceCells();
}

void CellShifting::shiftCellsInsideRows(util::MultiBox area, std::vector<circuit::Cell> &cells)
{
    mCellSlices.clear();

    mSubrows.createSubrows(area);

    sliceCells(cells);

    float rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();
    float siteWidth = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().x();

    entity_system::Property<Subrow, std::vector<CellSlice>> subrowsSlices(mSubrows.makeProperty<std::vector<CellSlice>>(Subrow()));
    for (auto cell : cells) {
        if (!mDesign.placement().isFixed(cell)) {
            auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];

            geometry::Box sliceBox(cellGeometry.min_corner(), {cellGeometry.max_corner().x(), cellGeometry.min_corner().y() + rowHeight});
            for (auto cellSlice : mCircuitCellsSlices[cell]) {
                auto subrow = mSubrows.findContainedSubrow(sliceBox);
                subrowsSlices[subrow].push_back(cellSlice);
                geometry::Box translatedBox;
                geometry::translate(sliceBox, 0, rowHeight, translatedBox);
                sliceBox = translatedBox;
            }
        }
    }

    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    GRBQuadExpr objectiveFunction;
    entity_system::Property<CellSlice, GRBVar> sliceVariables(mCellSlices);
    for (auto subrow : mSubrows.range(Subrow())) {
        auto subrowCellSlices = subrowsSlices[subrow];
        auto subrowOrigin = mSubrows.origin(subrow).toPoint();
        auto subrowUpperCorner = mSubrows.upperCorner(subrow).toPoint();

        std::vector<std::pair<CellSlice, util::micrometer_t>> sortedCellSlices;
        sortedCellSlices.reserve(subrowCellSlices.size());
        for (auto slice : subrowCellSlices) {
            auto sliceCircuitCell = mSlice2Cell[slice];
            auto cellLocation = mCellsInitialLocations[sliceCircuitCell];
            sortedCellSlices.push_back(std::make_pair(slice, cellLocation.x()));
        }
        std::sort(sortedCellSlices.begin(), sortedCellSlices.end(), CellSlicePairComparator());

        for (std::size_t sliceIndex = 0; sliceIndex < sortedCellSlices.size(); sliceIndex++) {
            auto slice = sortedCellSlices[sliceIndex].first;
            auto circuitCell = mSlice2Cell[slice];
            auto cellGeometry = mDesign.placementMapping().geometry(circuitCell)[0];
            auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();

            auto cellName = mSliceNames[slice];
            std::string variableName = cellName + "_x_variable";
            auto sliceCircuitCell = mSlice2Cell[slice];
            auto sliceInitialX = mCellsInitialLocations[sliceCircuitCell].toPoint().x();
//            GRBVar sliceXVariable = model.addVar(subrowOrigin.x() / siteWidth, (subrowUpperCorner.x() - cellWidth) / siteWidth, subrowOrigin.x() / siteWidth, GRB_CONTINUOUS, variableName);
            GRBVar sliceXVariable = model.addVar(subrowOrigin.x(), (subrowUpperCorner.x() - cellWidth), sliceInitialX, GRB_CONTINUOUS, variableName);
            sliceVariables[slice] = sliceXVariable;

//            objectiveFunction += (sliceXVariable * siteWidth - sliceInitialX) * (sliceXVariable * siteWidth - sliceInitialX);
            objectiveFunction += (sliceXVariable - sliceInitialX) * (sliceXVariable - sliceInitialX);

            if (sliceIndex > 0) {
                auto previousSlice = sortedCellSlices[sliceIndex-1].first;
                auto previousSliceName = mSliceNames[previousSlice];
                auto previousSliceCell = mSlice2Cell[previousSlice];
                auto previousCellGeometry = mDesign.placementMapping().geometry(previousSliceCell)[0];
                auto previousSliceWidth = previousCellGeometry.max_corner().x() - previousCellGeometry.min_corner().x();
                auto previousSliceVariable = sliceVariables[previousSlice];

//                model.addConstr(previousSliceVariable + (previousSliceWidth / siteWidth) <= sliceXVariable, cellName + "_" + previousSliceName + "_overlap_constraint");
                model.addConstr(previousSliceVariable + previousSliceWidth <= sliceXVariable, cellName + "_" + previousSliceName + "_overlap_constraint");
            }

//            for (std::size_t slice2Index = 0; slice2Index < sliceIndex; slice2Index++) {
//                auto slice2 = sortedCellSlices[slice2Index].first;
//                auto slice2Name = mSliceNames[slice2];
//                GRBVar leftVariable = model.addVar(0, 1, 0, GRB_BINARY, cellName + "_" + slice2Name + "_left_variable");
//                GRBVar rightVariable = model.addVar(0, 1, 0, GRB_BINARY, cellName + "_" + slice2Name + "_right_variable");

//                auto slice2Cell = mSlice2Cell[slice2];
//                auto slice2CellGeometry = mDesign.placementMapping().geometry(slice2Cell)[0];
//                auto slice2Width = slice2CellGeometry.max_corner().x() - slice2CellGeometry.min_corner().x();
//                auto slice2Variable = sliceVariables[slice2];

//                model.addConstr(slice2Variable + slice2Width - (1-leftVariable)*subrowUpperCorner.x() <= sliceXVariable, cellName + "_" + slice2Name + "_left_constraint");
//                model.addConstr(sliceXVariable + cellWidth - (1-rightVariable)*subrowUpperCorner.x() <= slice2Variable, cellName + "_" + slice2Name + "_right_constraint");
//                model.addConstr(leftVariable + rightVariable == 1, cellName + "_" + slice2Name + "_overlap_constraint");
//            }
        }
    }

    unsigned sliceForce = mDesign.floorplan().chipUpperRightCorner().toPoint().x();
    for (auto cell : cells) {
        auto cellSlices = mCircuitCellsSlices[cell];
        for (unsigned sliceIndex = 1; sliceIndex < cellSlices.size(); sliceIndex++) {
            auto currentSlice = cellSlices[sliceIndex];
            auto currentSliceVariable = sliceVariables[currentSlice];
            auto currentSliceName = mSliceNames[currentSlice];

            auto previousSlice = cellSlices[sliceIndex-1];
            auto previousSliceVariable = sliceVariables[previousSlice];
            auto previousSliceName = mSliceNames[previousSlice];

//            model.addConstr(currentSliceVariable == previousSliceVariable, currentSliceName + "_" + previousSliceName + "_slice_constraint");
//            objectiveFunction += (currentSliceVariable - previousSliceVariable)*(currentSliceVariable - previousSliceVariable)*sliceForce;
        }
    }

    model.setObjective(objectiveFunction, GRB_MINIMIZE);
    model.optimize();

    for (auto cellSlice : mCellSlices) {
        auto circuitCell = mSlice2Cell[cellSlice];
        if (mCircuitCellsSlices[circuitCell][0] == cellSlice) {
            auto sliceVariable = sliceVariables[cellSlice];
            auto currentCellLocation = mDesign.placement().cellLocation(circuitCell);
//            auto cellXLocation = std::round(sliceVariable.get(GRB_DoubleAttr_X)) * siteWidth;
            auto cellXLocation = std::round(sliceVariable.get(GRB_DoubleAttr_X) / siteWidth) * siteWidth;
            mDesign.placement().placeCell(circuitCell, util::Location(cellXLocation, currentCellLocation.toPoint().y()));
        }
    }
}

void CellShifting::sliceCells(std::vector<circuit::Cell> & cells)
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

            auto cellName = mDesign.netlist().name(cell);
            std::string sliceName = cellName + "_slice" + boost::lexical_cast<std::string>(sliceIndex);
            mSliceNames[cellSlice] = sliceName;
        }
    }
}
}
}
