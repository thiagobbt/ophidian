#ifndef CELLSHIFTING_H
#define CELLSHIFTING_H

#include <gurobi_c++.h>

#include <ophidian/legalization/Subrows.h>
#include <ophidian/legalization/FenceRegionIsolation.h>
#include <ophidian/legalization/RowAssignment.h>

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {
class CellShifting
{
public:
    using RtreeNode = std::pair<geometry::Box, circuit::Cell>;
    using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16> >;

    class CellSlicePairComparator
    {
    public:
        bool operator()(const std::pair<CellSlice, util::micrometer_t> & cellPair1, const std::pair<CellSlice, util::micrometer_t> & cellPair2) {
            return cellPair1.second < cellPair2.second;
        }
    };

    CellShifting(design::Design & design);

    void shiftCellsInsideRows();

private:
    void shiftCellsInsideRows(util::MultiBox area, std::vector<circuit::Cell> & cells);

    void sliceCells(std::vector<circuit::Cell> & cells);

    design::Design & mDesign;

    Subrows mSubrows;

    entity_system::Property<circuit::Cell, util::Location> mCellsInitialLocations;

    entity_system::EntitySystem<CellSlice> mCellSlices;
    entity_system::Property<circuit::Cell, std::vector<CellSlice>> mCircuitCellsSlices;
    entity_system::Property<CellSlice, circuit::Cell> mSlice2Cell;
    entity_system::Property<CellSlice, placement::RowAlignment> mSliceAlignment;
    entity_system::Property<CellSlice, std::string> mSliceNames;
};
}
}


#endif // CELLSHIFTING_H
