#include "GreedyRowAssignment.h"

namespace ophidian {
namespace legalization {
GreedyRowAssignment::GreedyRowAssignment(design::Design & design)
    : mDesign(design), mSubrows(design.netlist(), design.floorplan(), design.placement(), design.placementMapping())
{

}

void GreedyRowAssignment::assignCellsToRows()
{
    FenceRegionIsolation fenceRegionIsolation(mDesign);

    for(auto fence : mDesign.fences().range())
    {
        std::vector<circuit::Cell> cells (mDesign.fences().members(fence).begin(), mDesign.fences().members(fence).end());
        assignCellsToRows(mDesign.fences().area(fence), cells);
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

void GreedyRowAssignment::assignCellsToRows(util::MultiBox area, std::vector<circuit::Cell> &cells)
{
    mSubrows.createSubrows(cells, area);

    entity_system::Property<Subrow, util::micrometer_t> subrowsCapacities(mSubrows.makeProperty<util::micrometer_t>(Subrow()));
    for (auto subrow : mSubrows.range(Subrow())) {
        auto capacity = mSubrows.capacity(subrow);
        subrowsCapacities[subrow] = capacity;
    }

    for (auto cell : cells) {
        auto cellLocation = mDesign.placement().cellLocation(cell);
        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
        if (!mDesign.placement().isFixed(cell)) {
            std::vector<SubrowAssignment> subrowAssignments;
            subrowAssignments.reserve(mSubrows.rowCount());
            for (auto subrow : mSubrows.range(Subrow())) {
                if (cellWidth <= units::unit_cast<double>(subrowsCapacities[subrow])) {
                    auto subrowOrigin = mSubrows.origin(subrow);
                    double displacement = cellLocation.toPoint().y() - subrowOrigin.toPoint().y();
                    subrowAssignments.push_back(std::make_pair(subrow, displacement));
                }
            }

            Subrow assignedSubrow;
            std::sort(subrowAssignments.begin(), subrowAssignments.end(), SubrowAssignmentComparator());
            for (auto subrowAssignment : subrowAssignments) {
                auto subrowCapacity = mSubrows.capacity(subrowAssignment.first);
                if (cellWidth <= units::unit_cast<double>(subrowCapacity)) {
                    assignedSubrow = subrowAssignment.first;
                    break;
                }
            }

            auto assignedSubrowOrigin = mSubrows.origin(assignedSubrow);
            mDesign.placement().placeCell(cell, assignedSubrowOrigin);
        }
    }
}
}
}
