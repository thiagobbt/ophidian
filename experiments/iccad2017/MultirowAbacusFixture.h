#ifndef MULTIROWABACUSFIXTURE_H
#define MULTIROWABACUSFIXTURE_H

#include <ophidian/legalization/MultirowAbacus.h>
#include <fstream>

class MultiRowAbacusFixture : public ophidian::legalization::MultirowAbacus
{
public:
    MultiRowAbacusFixture(const ophidian::circuit::Netlist & netlist, const ophidian::floorplan::Floorplan & floorplan, ophidian::placement::Placement & placement, const ophidian::placement::PlacementMapping & placementMapping);

    void writeCsvWithCellsPerSubrow(std::string fileName);
};

#endif // MULTIROWABACUSFIXTURE_H
