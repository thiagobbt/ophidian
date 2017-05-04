#ifndef ABACUS_H
#define ABACUS_H

#include <ophidian/placement/PlacementMapping.h>
#include <ophidian/floorplan/Floorplan.h>
#include <ophidian/geometry/Distance.h>

namespace ophidian {
namespace legalization {

class Subrow : public entity_system::EntityBase
{
    public:
        using entity_system::EntityBase::EntityBase;
};

class AbacusCell : public entity_system::EntityBase
{
    public:
        using entity_system::EntityBase::EntityBase;
};

class Cluster : public entity_system::EntityBase
{
    public:
        using entity_system::EntityBase::EntityBase;
};

using RtreeNode = std::pair<geometry::Box, Subrow>;
using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16>>;

class Abacus
{
public:
    class CellPairComparator {
    public:
        bool operator()(const std::pair<AbacusCell, util::Location> & cellPair1, const std::pair<AbacusCell, util::Location> & cellPair2) {
            return cellPair1.second.x() < cellPair2.second.x();
        }
    };

    Abacus(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping);

    void legalizePlacement();

    void placeRow(Subrow subrow);

    void addCell(entity_system::Association<Subrow, AbacusCell>::Parts::PartIterator abacusCellIt, Cluster cluster);

    void addCluster(Cluster cluster, Cluster nextCluster);

    void collapse(std::vector<Cluster>::const_iterator clusterIt, Subrow subrow);

    void createSubrows();

private:
    const circuit::Netlist & netlist_;
    const floorplan::Floorplan & floorplan_;
    placement::Placement & placement_;
    const placement::PlacementMapping & placementMapping_;

    entity_system::EntitySystem<Subrow> subrows_;
    entity_system::Property<Subrow, util::Location> subrowOrigins_;
    entity_system::Property<Subrow, util::Location> subrowUpperCorners_;
    entity_system::Aggregation<Subrow, AbacusCell> subrowCells_;

    entity_system::EntitySystem<AbacusCell> abacusCells_;
    entity_system::Property<AbacusCell, circuit::Cell> abacusCell2NetlistCell_;
    entity_system::Property<AbacusCell, util::Location> cellInitialLocations_;
    entity_system::Property<AbacusCell, util::Location> cellLegalLocations_;
    entity_system::Property<AbacusCell, ophidian::util::micrometer_t> cellWidths_;
    entity_system::Property<AbacusCell, double> cellWeights_;

    entity_system::EntitySystem<Cluster> clusters_;
    entity_system::Property<Cluster, ophidian::util::micrometer_t> clusterOrigins_;
    entity_system::Property<Cluster, ophidian::util::micrometer_t> clusterWidths_;
    entity_system::Property<Cluster, double> clusterWeights_;
    entity_system::Property<Cluster, ophidian::util::micrometer_t> clusterDisplacements_;
    entity_system::Property<Cluster, entity_system::Association<Subrow, AbacusCell>::Parts::PartIterator> clusterLastCells_;

    Rtree subrowsRtree_;
};
}
}

#endif // ABACUS_H
