#ifndef MULTIROWLEGALIZATION_H
#define MULTIROWLEGALIZATION_H

#include <ophidian/design/Design.h>

#include <ophidian/legalization/Subrows.h>

#include <ophidian/legalization/LegalizationCheck.h>

namespace ophidian {
namespace legalization {
class MultirowLegalization
{
public:
    class CellPairComparator
    {
public:
        bool operator()(const std::pair<circuit::Cell, util::micrometer_t> & cellPair1, const std::pair<circuit::Cell, util::micrometer_t> & cellPair2) {
            return cellPair1.second < cellPair2.second;
        }
    };

    class Cluster : public entity_system::EntityBase
    {
        public:
            using entity_system::EntityBase::EntityBase;
    };

    using RtreeNode = std::pair<geometry::Box, circuit::Cell>;
    using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16> >;

public:
    MultirowLegalization(design::Design & design);

    void legalize(std::vector<circuit::Cell> cells, ophidian::util::MultiBox legalizationArea);

    util::micrometer_t legalizeCell(circuit::Cell cell, util::Location targetLocation, bool trial);

    void addCellToCluster(circuit::Cell cell, Cluster cluster, util::Location targetLocation);

    void moveClusterToOptimalLocation(Cluster cluster);

    void mergeClusters(std::vector<Cluster> clusters);

    util::micrometer_t clusterLeftSpace(Cluster cluster);

    void initialize(const std::vector<circuit::Cell> & cells, const ophidian::util::MultiBox & legalizationArea);

private:
    design::Design & mDesign;

    Subrows mSubrows;

    entity_system::EntitySystem<Cluster> mClusters;
    entity_system::Property<Cluster, std::vector<circuit::Cell>> mClustersCells;
    entity_system::Property<Cluster, std::map<util::micrometer_t, util::micrometer_t>> mClustersRightBoundary;

    entity_system::Property<circuit::Cell, Cluster> mCell2Cluster;
    entity_system::Property<circuit::Cell, Subrow> mCell2Subrow;

    entity_system::Property<Subrow, util::micrometer_t> mSubrowsRightBoundary;

    Rtree mCellsRtree;

    entity_system::Property<circuit::Cell, util::Location> mInitialLocations;
    entity_system::Property<circuit::Cell, util::Location> mLegalLocations;

    bool mDebug;
};
}
}


#endif // MULTIROWLEGALIZATION_H
