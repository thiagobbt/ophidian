#include "RtreeLegalization.h"

namespace ophidian {
namespace legalization {
RtreeLegalization::RtreeLegalization(design::Design &design)
    : mDesign(design),
      mCellsInitialLocations(design.netlist().makeProperty<util::Location>(circuit::Cell())),
      mCell2Subrow(design.netlist().makeProperty<Subrow>(circuit::Cell())),
      mSubrows(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping()) {

}

void RtreeLegalization::legalizePlacement(std::vector<circuit::Cell> &cells, util::MultiBox &legalizationArea)
{
    mSubrows.createSubrows(legalizationArea);

    for (auto subrow : mSubrows.range(Subrow())) {
        auto subrowOrigin = mSubrows.origin(subrow);
    }
    std::cout << std::endl;

    mCellsRtree.clear();

    auto siteWidth = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).x();
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).y();
    for (auto cell : cells) {
        if (!mDesign.placement().isFixed(cell)) {
            auto currentLocation = mDesign.placement().cellLocation(cell);

            auto cellBox = mDesign.placementMapping().geometry(cell)[0];
            auto cellWidth = cellBox.max_corner().x() - cellBox.min_corner().x();
            auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();

            std::vector<Subrow> closestSubrows;
            mSubrows.findClosestSubrows(1, currentLocation, closestSubrows);
            auto subrow = closestSubrows.front();
            auto subrowOrigin = mSubrows.origin(subrow);
            auto subrowUpperCorner = mSubrows.upperCorner(subrow);

            auto alignedX = std::floor(units::unit_cast<double>(currentLocation.x()) / units::unit_cast<double>(siteWidth)) * siteWidth;
            alignedX = std::max(alignedX, subrowOrigin.x());
            alignedX = std::min(alignedX, subrowUpperCorner.x() - util::micrometer_t(cellWidth));

            util::Location alignedLocation(alignedX, subrowOrigin.y());
            mDesign.placement().placeCell(cell, alignedLocation);

            geometry::Box newBox(alignedLocation.toPoint(), geometry::Point(alignedLocation.toPoint().x() + cellWidth, alignedLocation.toPoint().y() + cellHeight));
            mCellsRtree.insert(RtreeNode(newBox, cell));

            auto containedSubrow = mSubrows.findContainedSubrow(newBox);
            mCell2Subrow[cell] = containedSubrow;
        }
    }

    for (auto cell : cells) {
        if (!mDesign.placement().isFixed(cell)) {
            auto targetLocation = mDesign.placement().cellLocation(cell);
            while (!legalizeCell(cell, targetLocation)) {
                targetLocation.y(targetLocation.y() + rowHeight);
            }
        }
    }
}

void RtreeLegalization::legalizeCell(circuit::Cell cell)
{
    util::micrometer_t chipTop = mDesign.floorplan().chipUpperRightCorner().y();

    // try subrows until finding an available one
    unsigned rowsToSearch = 5;
    bool legalized = false;

    auto cellLocation = mDesign.placement().cellLocation(cell);
    auto cellBox = mDesign.placementMapping().geometry(cell)[0];
    auto cellWidth = util::micrometer_t(cellBox.max_corner().x() - cellBox.min_corner().x());
    auto cellHeight = util::micrometer_t(cellBox.max_corner().y() - cellBox.min_corner().y());

    while (!legalized)
    {
        std::vector<Subrow> closeSubrows;
        closeSubrows.reserve(rowsToSearch);
        mSubrows.findClosestSubrows(rowsToSearch, cellLocation, closeSubrows);
        for (auto subrow : closeSubrows)
        {
            auto capacity = mSubrows.capacity(subrow);
            auto origin = mSubrows.origin(subrow).y();
            if ((mSubrows.capacity(subrow) >= cellWidth) && ((mSubrows.origin(subrow).y() + cellHeight) <= chipTop))
            {
                util::Location targetLocation(cellLocation.x(), origin);
                legalized = legalizeCell(cell, targetLocation);
            }
        }
        rowsToSearch *= 2;
        if (rowsToSearch > mSubrows.rowCount()) {
            break;
        }
    }

    assert(legalized);
}

bool RtreeLegalization::legalizeCell(circuit::Cell cell, util::Location targetLocation)
{
    std::set<Overlap> overlaps;

    std::vector<std::pair<circuit::Cell, util::Location>> movements;
    movements.push_back(std::make_pair(cell, targetLocation));

    findOverlaps(cell, targetLocation, overlaps, OverlapDirection::ANY);

    while (!overlaps.empty()) {
        auto overlap = *overlaps.begin();
        overlaps.erase(overlaps.begin());

        auto fixedCell = overlap.firstCell;
        auto overlappingCell = overlap.secondCell;

        auto fixedCellBox = mDesign.placementMapping().geometry(fixedCell)[0];
        auto overlappingCellBox = mDesign.placementMapping().geometry(overlappingCell)[0];

        auto overlappingCellWidth = overlappingCellBox.max_corner().x() - overlappingCellBox.min_corner().x();

        auto subrow = mCell2Subrow[overlappingCell];
        auto subrowOrigin = mSubrows.origin(subrow);
        auto subrowUpperCorner = mSubrows.upperCorner(subrow);

        geometry::Point currentLocation = overlappingCellBox.min_corner();
        double movement;
        if (overlap.direction == LEFT) {
            movement = fixedCellBox.min_corner().x() - overlappingCellBox.max_corner().x();
        } else {
            movement = fixedCellBox.max_corner().x() - overlappingCellBox.min_corner().x();
        }

        util::Location newLocation(currentLocation.x() + movement, currentLocation.y());
        if ((newLocation.x() < subrowOrigin.x()) || (newLocation.x() + util::micrometer_t(overlappingCellWidth) > subrowUpperCorner.x())) {
            return false;
        }
        movements.push_back(std::make_pair(overlappingCell, newLocation));

        findOverlaps(overlappingCell, newLocation, overlaps, overlap.direction);
    }

    for (auto movement : movements) {
        auto currentCellBox = mDesign.placementMapping().geometry(movement.first)[0];
        RtreeNode oldNode(currentCellBox, movement.first);
        mCellsRtree.remove(oldNode);

        mDesign.placement().placeCell(movement.first, movement.second);
//        mDesign.placement().fixLocation(movement.first, true);

        auto newCellBox = mDesign.placementMapping().geometry(movement.first)[0];
        RtreeNode newNode(newCellBox, movement.first);
        mCellsRtree.insert(newNode);
    }

    return true;
}

void RtreeLegalization::buildRtree(std::vector<circuit::Cell> &cells, util::MultiBox legalizationArea)
{
    mSubrows.createSubrows(legalizationArea);

    mCellsRtree.clear();

    for (auto cell : cells) {
        auto cellBox = mDesign.placementMapping().geometry(cell)[0];
        mCellsRtree.insert(RtreeNode(cellBox, cell));

        auto subrow = mSubrows.findContainedSubrow(cellBox);
        mCell2Subrow[cell] = subrow;
    }
}

void RtreeLegalization::findOverlaps(circuit::Cell cell, util::Location targetLocation, std::set<Overlap> &overlaps, OverlapDirection previousDirection)
{
    auto cellBox = mDesign.placementMapping().geometry(cell)[0];
    auto cellWidth = cellBox.max_corner().x() - cellBox.min_corner().x();
    auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();

    geometry::Box targetBox(targetLocation.toPoint(), geometry::Point(targetLocation.toPoint().x() + cellWidth, targetLocation.toPoint().y() + cellHeight));

    std::vector<RtreeNode> intersectingCells;
    mCellsRtree.query(boost::geometry::index::intersects(targetBox), std::back_inserter(intersectingCells));
    for (auto intersectingCell : intersectingCells) {
        geometry::Box intersectionBox;
        boost::geometry::intersection(intersectingCell.first, targetBox, intersectionBox);
        double intersectionArea = boost::geometry::area(intersectionBox);
        if (intersectingCell.second != cell && intersectionArea > 0) {
            Overlap overlap;
            overlap.firstCell = cell;
            overlap.secondCell = intersectingCell.second;
            if (previousDirection == ANY) {
                auto leftMovement = targetBox.min_corner().x() - intersectingCell.first.max_corner().x();
                auto rightMovement = targetBox.max_corner().x() - intersectingCell.first.min_corner().x();
                overlap.direction = (std::abs(leftMovement) <= std::abs(rightMovement)) ? LEFT : RIGHT;
            } else {
                overlap.direction = previousDirection;
            }

            overlaps.insert(overlap);
        }
    }
}
}
}
