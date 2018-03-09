#include "RectilinearFences.h"

namespace ophidian {
namespace legalization {
RectilinearFences::RectilinearFences(design::Design &design)
    : mDesign(design), mFenceBlocks(design.fences().makeProperty<std::vector<circuit::Cell>>()) {

}

void RectilinearFences::addBlocksToRectilinearFences()
{
    for (auto fence : mDesign.fences().range()) {
        auto area = mDesign.fences().area(fence);
        if (area.size() > 1) {
            addBlockToFence(fence);
        }
    }
}

void RectilinearFences::addBlockToFence(placement::Fence fence)
{
    auto fenceName = mDesign.fences().name(fence);

    auto fenceArea = mDesign.fences().area(fence);
    geometry::Box fenceBoundingBox;
    boost::geometry::envelope(fenceArea.toMultiPolygon(), fenceBoundingBox);

    geometry::MultiPolygon difference;
    boost::geometry::difference(fenceBoundingBox, fenceArea.toMultiPolygon(), difference);

    unsigned blockIndex = 0;
    for (auto polygon : difference) {
        geometry::Box macroBlockBox;
        boost::geometry::envelope(polygon, macroBlockBox);

        std::string cellName = fenceName + "_block"+ boost::lexical_cast<std::string>(blockIndex++);
        auto stdCell = mDesign.standardCells().add(ophidian::standard_cell::Cell(), cellName);
        auto circuitCell = mDesign.netlist().add(ophidian::circuit::Cell(), cellName);

        geometry::Box traslatedBox;
        geometry::translate(macroBlockBox, -macroBlockBox.min_corner().x(), -macroBlockBox.min_corner().y(), traslatedBox);

        mDesign.library().geometry(stdCell, util::MultiBox({traslatedBox}));
        mDesign.library().cellAlignment(stdCell, placement::RowAlignment::NA);
        mDesign.placement().placeCell(circuitCell, util::Location(macroBlockBox.min_corner().x(), macroBlockBox.min_corner().y()));
        mDesign.placement().fixLocation(circuitCell, true);
        mDesign.libraryMapping().cellStdCell(circuitCell, stdCell);

        mFenceBlocks[fence].push_back(circuitCell);

        mDesign.fences().connect(fence, circuitCell);
    }
}

void RectilinearFences::eraseBlocks()
{
    for (auto fence : mDesign.fences().range()) {
        for (auto blockCell : mFenceBlocks[fence]) {
            mDesign.netlist().erase(blockCell);
        }
        mFenceBlocks[fence].clear();
    }
}

}
}
