#include "SeparateCellsIntoBoxes.h"

namespace ophidian {
namespace legalization {
SeparateCellsIntoBoxes::SeparateCellsIntoBoxes(design::Design &design)
    : mDesign(design), mAreas(mRegions), mFreeAreas(mRegions), mOccupiedAreas(mRegions), mRegionCells(mRegions, mDesign.netlist().cells_) {

}

void SeparateCellsIntoBoxes::separateCells(const std::vector<circuit::Cell> &cells, const util::MultiBox &area, double densityThreshold)
{
    int boxIndex = 0;
    for (auto box : area) {
        auto region = mRegions.add();
        mAreas[region] = box;
        mFreeAreas[region] = boost::geometry::area(box);

        auto regionName = "region_" + boost::lexical_cast<std::string>(boxIndex++);
        mName2Region[regionName] = region;

        mRegionsRtree.insert(RegionRtreeNode(box, region));
    }

    for (auto cell : cells) {
        if (mDesign.placement().isFixed(cell)) {
            auto cellBox = mDesign.placementMapping().geometry(cell)[0];
            for (auto region : mRegions) {
                geometry::Box intersectionBox;
                boost::geometry::intersection(cellBox, mAreas[region], intersectionBox);
                mFreeAreas[region] = mFreeAreas[region] - boost::geometry::area(intersectionBox);
            }
        }
    }

    int cellIndex = 0;
    for (auto cell : cells) {
        if (!mDesign.placement().isFixed(cell)) {
            if (!addToContainedRegion(cell, densityThreshold)) {
                if (!addToClosestRegion(cell, densityThreshold)) {
                    addToLeastDenseRegion(cell);
                }
            }
        }
        cellIndex++;
    }
}

double SeparateCellsIntoBoxes::distanceBetweenCellAndRegion(circuit::Cell cell, Region region)
{
    auto cellLocation = mDesign.placement().cellLocation(cell).toPoint();
    auto cellBox = mDesign.placementMapping().geometry(cell)[0];
    auto cellWidth = cellBox.max_corner().x() - cellBox.min_corner().x();
    auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();

    auto regionBox = mAreas[region];

    double distanceX = 0;
    if (cellLocation.x() < regionBox.min_corner().x()) {
        distanceX = regionBox.min_corner().x() - cellLocation.x();
    } else if (cellLocation.x() + cellWidth > regionBox.max_corner().x()) {
        distanceX = cellLocation.x() + cellWidth - regionBox.max_corner().x();
    }

    double distanceY = 0;
    if (cellLocation.y() < regionBox.min_corner().y()) {
        distanceY = regionBox.min_corner().y() - cellLocation.y();
    } else if (cellLocation.y() + cellHeight > regionBox.max_corner().y()) {
        distanceY = cellLocation.y() + cellHeight - regionBox.max_corner().y();
    }

    return distanceX + distanceY;
}

bool SeparateCellsIntoBoxes::addToContainedRegion(circuit::Cell cell, double densityThreshold)
{
    auto cellBox = mDesign.placementMapping().geometry(cell)[0];
    std::vector<RegionRtreeNode> foundNodes;
    mRegionsRtree.query(boost::geometry::index::contains(cellBox), std::back_inserter(foundNodes));
    if (!foundNodes.empty()) {
        assert(foundNodes.size() == 1);
        auto region = foundNodes.front().second;
        auto regionDensity = density(region);
        if (regionDensity <= densityThreshold) {
            mOccupiedAreas[region] = mOccupiedAreas[region] + boost::geometry::area(cellBox);
            mRegionCells.addAssociation(region, cell);
            return true;
        }
    }
    return false;
}

bool SeparateCellsIntoBoxes::addToClosestRegion(circuit::Cell cell, double densityThreshold)
{
    auto cellLocation = mDesign.placement().cellLocation(cell).toPoint();
    std::vector<std::pair<Region, double>> sortedRegions;
    sortedRegions.reserve(mRegions.size());
    for (auto region : mRegions) {
        auto regionLocation = mAreas[region].min_corner();
//        auto distance = std::abs(regionLocation.x() - cellLocation.x()) + std::abs(regionLocation.y() - cellLocation.y());
        auto distance = distanceBetweenCellAndRegion(cell, region);
        sortedRegions.push_back(std::make_pair(region, distance));
    }

    std::sort(sortedRegions.begin(), sortedRegions.end(), RegionPairComparator());

    for (auto region : sortedRegions) {
        auto regionDensity = density(region.first);
        if (regionDensity <= densityThreshold) {
            auto cellBox = mDesign.placementMapping().geometry(cell)[0];
            mOccupiedAreas[region.first] = mOccupiedAreas[region.first] + boost::geometry::area(cellBox);
            mRegionCells.addAssociation(region.first, cell);
            return true;
        }
    }

    return false;
}

void SeparateCellsIntoBoxes::addToLeastDenseRegion(circuit::Cell cell)
{
    auto cellBox = mDesign.placementMapping().geometry(cell)[0];

    std::vector<std::pair<Region, double>> sortedRegions;
    sortedRegions.reserve(mRegions.size());
    for (auto region : mRegions) {
        auto regionDensity = density(region);
        sortedRegions.push_back(std::make_pair(region, regionDensity));
    }

    std::sort(sortedRegions.begin(), sortedRegions.end(), RegionPairComparator());

    auto region = sortedRegions.front().first;
    mOccupiedAreas[region] = mOccupiedAreas[region] + boost::geometry::area(cellBox);
    mRegionCells.addAssociation(region, cell);
}
}
}
