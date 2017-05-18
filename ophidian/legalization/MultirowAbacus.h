#ifndef MULTIROWABACUS_H
#define MULTIROWABACUS_H

#include <ophidian/legalization/Abacus.h>

namespace ophidian {
namespace legalization {
class MultirowAbacus : public Abacus
{
public:
    MultirowAbacus(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping);

    void legalizePlacement();
};
}
}

#endif // MULTIROWABACUS_H
