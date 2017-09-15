#ifndef ABACUSPLACEROW_H
#define ABACUSPLACEROW_H

#include <vector>

#include <ophidian/util/Units.h>

#include <ophidian/legalization/Subrows.h>

namespace ophidian {
namespace legalization {

class Cluster : public entity_system::EntityBase
{
    public:
        using entity_system::EntityBase::EntityBase;
};


class AbacusCell : public entity_system::EntityBase
{
    public:
        using entity_system::EntityBase::EntityBase;
};

class AbacusPlaceRow {
public:
    AbacusPlaceRow(Subrows & subrows, entity_system::Property<AbacusCell, util::Location> & cellInitialLocations, entity_system::Property<AbacusCell, util::Location> & cellLegalLocations,
                   entity_system::Property<AbacusCell, ophidian::util::micrometer_t> & cellWidths, entity_system::Property<AbacusCell, double> & cellWeigths);

    void operator()(Subrow subrow, const std::vector<AbacusCell> &subrowCells, util::micrometer_t siteWidth);

private:
    void addCell(std::vector<AbacusCell>::const_iterator abacusCellIt, Cluster cluster);

    void addCluster(Cluster cluster, Cluster nextCluster);

    void collapse(std::vector<Cluster>::const_iterator clusterIt, Subrow subrow, util::micrometer_t siteWidth);

    Subrows & subrows_;

    entity_system::EntitySystem<Cluster> clusters_;
    entity_system::Property<Cluster, ophidian::util::micrometer_t> clusterOrigins_;
    entity_system::Property<Cluster, ophidian::util::micrometer_t> clusterWidths_;
    entity_system::Property<Cluster, double> clusterWeights_;
    entity_system::Property<Cluster, ophidian::util::micrometer_t> clusterDisplacements_;
    entity_system::Property<Cluster, std::vector<AbacusCell>::const_iterator> clusterFirstCells_;
    entity_system::Property<Cluster, std::vector<AbacusCell>::const_iterator> clusterLastCells_;

    entity_system::Property<AbacusCell, util::Location> & cellInitialLocations_;
    entity_system::Property<AbacusCell, util::Location> & cellLegalLocations_;
    entity_system::Property<AbacusCell, ophidian::util::micrometer_t> & cellWidths_;
    entity_system::Property<AbacusCell, double> & cellWeights_;
};
}
}

#endif // ABACUSPLACEROW_H
