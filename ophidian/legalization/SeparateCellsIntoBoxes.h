#ifndef SEPARATECELLSINTOBOXES_H
#define SEPARATECELLSINTOBOXES_H

#include <unordered_map>

#include <ophidian/design/Design.h>
#include <ophidian/geometry/Distance.h>

namespace ophidian {
namespace legalization {

class Region : public entity_system::EntityBase
{
public:
    using entity_system::EntityBase::EntityBase;
};

using RegionRtreeNode = std::pair<geometry::Box, Region>;
using RegionRtree = boost::geometry::index::rtree<RegionRtreeNode, boost::geometry::index::rstar<16> >;
using Cells = entity_system::Association<Region, circuit::Cell>::Parts;

class RegionPairComparator
{
public:
    bool operator()(const std::pair<Region, double> & regionPair1, const std::pair<Region, double> & regionPair2) {
        return regionPair1.second < regionPair2.second;
    }
};

class SeparateCellsIntoBoxes
{
public:
    SeparateCellsIntoBoxes(ophidian::design::Design & design);

    void separateCells(const std::vector<circuit::Cell> & cells, const util::MultiBox & area, double densityThreshold = 0.9);

    double density(Region region) {
        return mOccupiedAreas[region] / mFreeAreas[region];
    }

    geometry::Box area(Region region) {
        return mAreas[region];
    }

    Region cellRegion(circuit::Cell cell) {
        return mRegionCells.whole(cell);
    }

    Cells regionCells(Region region) {
        return mRegionCells.parts(region);
    }

    Region find(std::string name) {
        return mName2Region[name];
    }

private:    
    double distanceBetweenCellAndRegion(circuit::Cell cell, Region region);

    bool addToContainedRegion(circuit::Cell cell, double densityThreshold);

    bool addToClosestRegion(circuit::Cell cell, double densityThreshold);

    void addToLeastDenseRegion(circuit::Cell cell);

    design::Design & mDesign;

    entity_system::EntitySystem<Region> mRegions;

    std::unordered_map<std::string, Region> mName2Region;

    entity_system::Property<Region, geometry::Box> mAreas;
    entity_system::Property<Region, double> mFreeAreas;
    entity_system::Property<Region, double> mOccupiedAreas;
    entity_system::Aggregation<Region, circuit::Cell> mRegionCells;

    RegionRtree mRegionsRtree;

};
}
}


#endif // SEPARATECELLSINTOBOXES_H
