#include "LegalizationFixing.h"

namespace ophidian {
namespace legalization {
LegalizationFixing::LegalizationFixing(design::Design &design)
    : mDesign(design),
      mSubrows(design.netlist(), design.floorplan(), design.placement(), design.placementMapping()),
      mInitialLocations(mDesign.netlist().makeProperty<util::Location>(circuit::Cell()))
{
    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); cellIt++) {
        mInitialLocations[*cellIt] = mDesign.placement().cellLocation(*cellIt);
    }
}

void LegalizationFixing::fixLegalization(util::MultiBox area, std::vector<circuit::Cell> & cells)
{
    mSubrows.createSubrows(area);

    double rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();
    double siteWidth = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().x();

    std::vector<circuit::Cell> legalizedCells;
    legalizedCells.reserve(mDesign.netlist().size(circuit::Cell()));
    std::vector<circuit::Cell> unlegalizedCells;
    unlegalizedCells.reserve(mDesign.netlist().size(circuit::Cell()));

    for (auto cell : cells) {
        auto cellLocation = mDesign.placement().cellLocation(cell);
        bool rowAlignment = (int)cellLocation.toPoint().y() % (int)rowHeight == 0;
        bool siteAlignment = (int)cellLocation.toPoint().x() % (int)siteWidth == 0;
        if (!rowAlignment || !siteAlignment) {
            unlegalizedCells.push_back(cell);
        } else {
            legalizedCells.push_back(cell);

            auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
            auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
            auto subrow = mSubrows.findContainedSubrow(cellGeometry);
            auto currentCapacity = mSubrows.capacity(subrow);
            mSubrows.capacity(subrow, currentCapacity - util::micrometer_t(cellWidth));
        }
    }
    legalizedCells.shrink_to_fit();
    unlegalizedCells.shrink_to_fit();

    std::vector<circuit::Cell> cellsToFix = legalizedCells;
    for (auto unlegalizedCell : unlegalizedCells) {
        cellsToFix.push_back(unlegalizedCell);

        auto cellLocation = mDesign.placement().cellLocation(unlegalizedCell);
        auto cellGeometry = mDesign.placementMapping().geometry(unlegalizedCell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        bool found = false;
        unsigned rowsToSearch = 5;
        while (!found)
        {
            std::vector<Subrow> closeSubrows;
            closeSubrows.reserve(rowsToSearch);
            mSubrows.findClosestSubrows(rowsToSearch, cellLocation, closeSubrows);
            for (auto subrow : closeSubrows)
            {
                auto capacity = units::unit_cast<double>(mSubrows.capacity(subrow));
                auto subrowY = mSubrows.origin(subrow).y();
                if ((capacity >= cellWidth))
                {
                    CellShifting cellShifting(mDesign);
                    cellShifting.shiftCellsInsideRows(area, cellsToFix);
                    auto cellLegalLocation = mDesign.placement().cellLocation(unlegalizedCell);
                    if (cellLegalLocation != cellLocation) {
                        found = true;
                        break;
                    }
                }
            }
            if (rowsToSearch == mSubrows.rowCount()) {
                std::cout << "could not legalize cell!" << std::endl;
                return;
            } else {
                rowsToSearch *= 2;
                if (rowsToSearch > mSubrows.rowCount()) {
                    rowsToSearch = mSubrows.rowCount();
                }
            }
        }
    }
}

}
}
