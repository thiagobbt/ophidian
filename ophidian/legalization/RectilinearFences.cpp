#include "RectilinearFences.h"

namespace ophidian {
namespace legalization {
RectilinearFences::RectilinearFences(design::Design &design)
    : mDesign(design), mFenceBlocks(design.fences().makeProperty<std::vector<circuit::Cell>>()), mIsolateFences(design) {

}

void RectilinearFences::addBlocksToRectilinearFences()
{
    for (auto fence : mDesign.fences().range()) {
        auto name = mDesign.fences().name(fence);
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

    Subrows subrows(mDesign);
    mIsolateFences.addFenceBlocks(fence);

    std::vector<circuit::Cell> cells(mDesign.fences().members(fence).begin(), mDesign.fences().members(fence).end());
    subrows.createSubrows(cells, util::MultiBox({fenceBoundingBox}), 1, placement::RowAlignment::NA, false);

    Rtree subrowsRtree;

    unsigned blockIndex = 0;
    for (auto subrow : subrows.range(Subrow())) {
        auto subrowOrigin = subrows.origin(subrow);
        auto subrowUpperCorner = subrows.upperCorner(subrow);
        geometry::Box subrowBox(subrowOrigin.toPoint(), subrowUpperCorner.toPoint());

//        std::vector<RtreeNode> intersectingSubrows;
//        subrowsRtree.query(boost::geometry::index::intersects(subrowBox), std::back_inserter(intersectingSubrows));
//        if (intersectingSubrows.size() > 0) {
//            for (auto node : intersectingSubrows) {
//                auto otherBox = node.first;
//                auto otherSubrow = node.second;
//                if (otherBox.min_corner.x() == subrowBox.min_corner().x() && otherBox.max_corner.x() == subrowBox.max_corner().x()) {
//                    mergeSubrows(subrow, otherSubrow);
//                }
//            }
//        } else {
//            std::string cellName = fenceName + "_block"+ boost::lexical_cast<std::string>(blockIndex++);
//            auto stdCell = mDesign.standardCells().add(ophidian::standard_cell::Cell(), cellName);
//            auto circuitCell = mDesign.netlist().add(ophidian::circuit::Cell(), cellName);

//            geometry::Box traslatedBox;
//            geometry::translate(subrowBox, -subrowBox.min_corner().x(), -subrowBox.min_corner().y(), traslatedBox);

//            mDesign.library().geometry(stdCell, util::MultiBox({traslatedBox}));
//            mDesign.library().cellAlignment(stdCell, placement::RowAlignment::NA);
//            mDesign.placement().placeCell(circuitCell, util::Location(subrowBox.min_corner().x(), subrowBox.min_corner().y()));
//            mDesign.placement().fixLocation(circuitCell, true);
//            mDesign.libraryMapping().cellStdCell(circuitCell, stdCell);

//            mFenceBlocks[fence].push_back(circuitCell);
//            mSubrow2Block[subrow] = circuitCell;
//        }

//        RtreeNode subrowNode(subrowBox, subrow);
//        subrowsRtree.insert(subrowNode);
        std::string cellName = fenceName + "_block"+ boost::lexical_cast<std::string>(blockIndex++);
        auto stdCell = mDesign.standardCells().add(ophidian::standard_cell::Cell(), cellName);
        auto circuitCell = mDesign.netlist().add(ophidian::circuit::Cell(), cellName);

        geometry::Box traslatedBox;
        geometry::translate(subrowBox, -subrowBox.min_corner().x(), -subrowBox.min_corner().y(), traslatedBox);

        mDesign.library().geometry(stdCell, util::MultiBox({traslatedBox}));
        mDesign.library().cellAlignment(stdCell, placement::RowAlignment::NA);
        mDesign.placement().placeCell(circuitCell, util::Location(subrowBox.min_corner().x(), subrowBox.min_corner().y()));
        mDesign.placement().fixLocation(circuitCell, true);
        mDesign.libraryMapping().cellStdCell(circuitCell, stdCell);

        mFenceBlocks[fence].push_back(circuitCell);
        mDesign.fences().connect(fence, circuitCell);
//        mSubrow2Block[subrow] = circuitCell;
    }

    mIsolateFences.eraseFenceBlocks(fence);
}

void RectilinearFences::eraseBlocks()
{
    for (auto fence : mDesign.fences().range()) {
        for (auto blockCell : mFenceBlocks[fence]) {
            mDesign.netlist().erase(blockCell);
        }
    }
}

void RectilinearFences::mergeSubrows(Subrow subrow1, Subrow subrow2)
{
//    auto block1 = mSubrow2Block[subrow1];
//    auto block2 = mSubrow2Block[subrow2];

//    if (block1 == circuit::Cell()) {
//        block1 = block2;
//    } else if (block2 == circuit::Cell()) {
//        block2 = block1;
//    }

//    if (block1 == block2) {

//    } else {
//        mDesign.netlist().erase(block1);
//        block1 = block2;
//    }
}

}
}

