#ifndef ABACUS_H
#define ABACUS_H

#include <ophidian/geometry/Distance.h>

#include <ophidian/legalization/AbacusPlaceRow.h>

namespace ophidian
{
namespace legalization
{

class Abacus
{
public:
    class CellPairComparator
    {
public:
        bool operator()(const std::pair<AbacusCell, util::Location> & cellPair1, const std::pair<AbacusCell, util::Location> & cellPair2) {
            return cellPair1.second.x() < cellPair2.second.x();
        }
    };

    Abacus(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping);

    virtual void legalizePlacement();

protected:
    void legalize(const std::vector<std::pair<AbacusCell, util::Location> > &sortedCells);

    const circuit::Netlist & netlist_;
    const floorplan::Floorplan & floorplan_;
    placement::Placement & placement_;
    const placement::PlacementMapping & placementMapping_;

    entity_system::EntitySystem<AbacusCell> abacusCells_;
    entity_system::Property<AbacusCell, circuit::Cell> abacusCell2NetlistCell_;
    entity_system::Property<AbacusCell, util::Location> cellInitialLocations_;
    entity_system::Property<AbacusCell, util::Location> cellLegalLocations_;
    entity_system::Property<AbacusCell, ophidian::util::micrometer_t> cellWidths_;
    entity_system::Property<AbacusCell, ophidian::util::micrometer_t> cellHeights_;
    entity_system::Property<AbacusCell, double> cellWeights_;

    Subrows subrows_;

    entity_system::Property<Subrow, std::vector<AbacusCell> > subrowCells_;

    AbacusPlaceRow abacusPlaceRow_;
};

} // namespace legalization
} // namespace ophidian

#endif // ABACUS_H
