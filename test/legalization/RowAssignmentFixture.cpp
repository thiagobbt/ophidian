#include "RowAssignmentFixture.h"

RowAssignmentFixture::RowAssignmentFixture(ophidian::design::Design &design)
    : ophidian::legalization::MixedRowAssignment(design),
      mSubrowsCells(mSubrows.makeProperty<std::vector<ophidian::circuit::Cell>>(ophidian::legalization::Subrow())) {

}

void RowAssignmentFixture::assignCellsToRows()
{
    MixedRowAssignment::assignCellsToRows();

    for (auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        auto cellBox = mDesign.placementMapping().geometry(*cellIt)[0];
        auto subrow = mSubrows.findContainedSubrow(cellBox);

        mSubrowsCells[subrow].push_back(*cellIt);
    }
}

bool RowAssignmentFixture::checkSubrowsCapacities()
{
    for (auto subrow : mSubrows.range(ophidian::legalization::Subrow())) {
        auto subrowCapacity = mSubrows.capacity(subrow);

        double occupiedSpace(0);
        for (auto cell : mSubrowsCells[subrow]) {
            auto cellBox = mDesign.placementMapping().geometry(cell)[0];
            auto cellWidth = cellBox.max_corner().x() - cellBox.min_corner().x();
            occupiedSpace += cellWidth;
        }

        if (occupiedSpace > units::unit_cast<double>(subrowCapacity)) {
            return false;
        }
    }
    return true;
}
