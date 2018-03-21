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
    addFenceBlocks(fence);
    fixCellsInFence(fence, true);
}

void FenceRegionIsolation::addFenceBlocks(placement::Fence fence, bool align)
{
    auto site = *mDesign.floorplan().sitesRange().begin();
    auto siteWidth = mDesign.floorplan().siteUpperRightCorner(site).toPoint().x();
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(site).toPoint().y();

    mFenceBlocks[fence].clear();
    util::MultiBox boxes = mDesign.fences().area(fence);
    for(auto box : boxes)
    {
        auto minCorner = box.min_corner();
        auto maxCorner = box.max_corner();
        minCorner.x(std::ceil(minCorner.x() / siteWidth) * siteWidth);
        minCorner.y(std::ceil(minCorner.y() / rowHeight) * rowHeight);
        maxCorner.x(std::floor(maxCorner.x() / siteWidth) * siteWidth);
        maxCorner.y(std::floor(maxCorner.y() / rowHeight) * rowHeight);

        geometry::Box alignedBox(minCorner, maxCorner);

        std::string cellName = mDesign.fences().name(fence) + "Block"+ boost::lexical_cast<std::string>(mContBox);
        auto stdCell = mDesign.standardCells().add(ophidian::standard_cell::Cell(), cellName);
        auto circuitCell = mDesign.netlist().add(ophidian::circuit::Cell(), cellName);

        geometry::Box traslatedBox;
        if (align) {
            geometry::translate(alignedBox, -alignedBox.min_corner().x(), -alignedBox.min_corner().y(), traslatedBox);
        } else {
            geometry::translate(box, -box.min_corner().x(), -box.min_corner().y(), traslatedBox);
        }

        mDesign.library().geometry(stdCell, util::MultiBox({traslatedBox}));
        mDesign.library().cellAlignment(stdCell, placement::RowAlignment::NA);
        if (align) {
            mDesign.placement().placeCell(circuitCell, util::Location(alignedBox.min_corner().x(), alignedBox.min_corner().y()));
        } else {
            mDesign.placement().placeCell(circuitCell, util::Location(box.min_corner().x(), box.min_corner().y()));
        }
        mDesign.placement().fixLocation(circuitCell, true);
        mDesign.libraryMapping().cellStdCell(circuitCell, stdCell);

        mFenceBlocks[fence].push_back(circuitCell);
        mDesign.fences().connect(fence, circuitCell);
        mContBox++;
    }
}

void FenceRegionIsolation::restoreAllFenceCells()
{
    for (auto fence : mDesign.fences().range()) {
        restoreFenceCells(fence);
    }
}

void FenceRegionIsolation::eraseFenceBlocks(placement::Fence fence)
{
    for (auto cell : mFenceBlocks[fence]) {
        mDesign.netlist().erase(cell);
    }
    mFenceBlocks[fence].clear();
}

void FenceRegionIsolation::restoreFenceCells(placement::Fence fence)
{
    eraseFenceBlocks(fence);
    fixCellsInFence(fence, false);
}

void FenceRegionIsolation::fixCellsInFences(bool fix)
{
    for (auto fence : mDesign.fences().range()) {
        fixCellsInFence(fence, fix);
    }
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
