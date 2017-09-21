#include "FenceRegionIsolation.h"

namespace ophidian {
namespace legalization {
FenceRegionIsolation::FenceRegionIsolation(design::Design &design)
    : mDesign(design), mFenceBlocks(design.fences().makeProperty<std::vector<circuit::Cell>>()) {

}

void FenceRegionIsolation::isolateAllFenceCells()
{
    for (auto fence : mDesign.fences().range()) {
        isolateFenceCells(fence);
    }
}

void FenceRegionIsolation::isolateFenceCells(placement::Fence fence)
{
    util::MultiBox boxes = mDesign.fences().area(fence);
    for(auto box : boxes)
    {
        std::string cellName = mDesign.fences().name(fence) + "Block"+ boost::lexical_cast<std::string>(mContBox);
        auto stdCell = mDesign.standardCells().add(ophidian::standard_cell::Cell(), cellName);
        auto circuitCell = mDesign.netlist().add(ophidian::circuit::Cell(), cellName);

        geometry::Box traslatedBox;
        geometry::translate(box, -box.min_corner().x(), -box.min_corner().y(), traslatedBox);

        mDesign.library().geometry(stdCell, util::MultiBox({traslatedBox}));
        mDesign.library().cellAlignment(stdCell, placement::RowAlignment::NA);
        mDesign.placement().placeCell(circuitCell, util::Location(box.min_corner().x(), box.min_corner().y()));
        mDesign.placement().fixLocation(circuitCell, true);
        mDesign.libraryMapping().cellStdCell(circuitCell, stdCell);

        mFenceBlocks[fence].push_back(circuitCell);
        mContBox++;
    }

    fixCellsInFence(fence, true);
}

void FenceRegionIsolation::restoreAllFenceCells()
{
    for (auto fence : mDesign.fences().range()) {
        restoreFenceCells(fence);
    }
}

void FenceRegionIsolation::restoreFenceCells(placement::Fence fence)
{
    for (auto cell : mFenceBlocks[fence]) {
        mDesign.netlist().erase(cell);
    }
    mFenceBlocks[fence].clear();
    fixCellsInFence(fence, false);
}

void FenceRegionIsolation::fixCellsInFence(placement::Fence fence, bool fix)
{
    for(circuit::Cell cell : mDesign.fences().members(fence))
    {
        mDesign.placement().fixLocation(cell, fix);
    }
}
}
}
