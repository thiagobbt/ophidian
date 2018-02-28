#ifndef MULTIROWABACUS_H
#define MULTIROWABACUS_H

#include <ophidian/legalization/Abacus.h>

namespace ophidian
{
namespace legalization
{
class MultirowAbacus : public Abacus
{
public:
    MultirowAbacus(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping);

    bool legalizePlacement(std::vector<circuit::Cell> cells, ophidian::util::MultiBox legalizationArea, double maxDisplacement = 0);
    bool legalizeSubrows(std::vector<circuit::Cell> &cellsForOneHeight, unsigned rowsPerCell, placement::RowAlignment alignment, util::MultiBox legalizationArea, double maxDisplacement = 0);
};
} // namespace legalization
} // namespace ophidian

#endif // MULTIROWABACUS_H
