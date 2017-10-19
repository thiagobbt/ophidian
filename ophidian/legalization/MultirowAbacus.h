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

    void legalizePlacement(std::vector<circuit::Cell> cells, ophidian::util::MultiBox legalizationArea);
    void legalizeSubrows(std::vector<circuit::Cell> &cellsForOneHeight, unsigned rowsPerCell, placement::RowAlignment alignment, util::MultiBox legalizationArea);
};
} // namespace legalization
} // namespace ophidian

#endif // MULTIROWABACUS_H
