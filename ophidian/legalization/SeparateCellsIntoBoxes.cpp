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

        auto regionName = "region_" + boost::lexical_cast<int>(boxIndex);
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

    for (auto cell : cells) {
        if (!mDesign.placement().isFixed(cell)) {
            if (!addToContainedRegion(cell, densityThreshold)) {
                if (!addToClosestRegion(cell, densityThreshold)) {
                    addToLeastDenseRegion(cell);
                }
            }
        }
    }
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
    geometry::ManhattanDistance manhattanDistance;

    auto cellLocation = mDesign.placement().cellLocation(cell).toPoint();
    std::vector<std::pair<Region, double>> sortedRegions;
    sortedRegions.reserve(mRegions.size());
    for (auto region : mRegions) {
        auto regionLocation = mAreas[region].min_corner();
        auto distance = std::abs(regionLocation.x() - cellLocation.x()) + std::abs(regionLocation.y() - cellLocation.y());
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
