#ifndef LEGALIZATIONFIXTURE_H
#define LEGALIZATIONFIXTURE_H

#include <ophidian/placement/PlacementMapping.h>
#include <ophidian/floorplan/Floorplan.h>

class LegalizationFixture
{
public:
    LegalizationFixture();

    void addCell(ophidian::standard_cell::Cell stdCell, ophidian::util::Location cellLocation, unsigned numberOfPins, bool fixed);

private:
    ophidian::standard_cell::StandardCells stdCells_;
    ophidian::placement::Library placementLibrary_;

    ophidian::circuit::Netlist netlist_;
    ophidian::circuit::LibraryMapping libraryMapping_;
    ophidian::floorplan::Floorplan floorplan_;
    ophidian::placement::Placement placement_;
    ophidian::placement::PlacementMapping placementMapping_;
};

#endif // LEGALIZATIONFIXTURE_H
