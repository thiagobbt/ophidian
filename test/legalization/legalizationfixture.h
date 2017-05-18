#ifndef LEGALIZATIONFIXTURE_H
#define LEGALIZATIONFIXTURE_H

#include <ophidian/placement/PlacementMapping.h>
#include <ophidian/floorplan/Floorplan.h>
#include <string>

class CircuitFixture
{
public:
    CircuitFixture();

    ophidian::circuit::Cell addCell(ophidian::standard_cell::Cell stdCell, ophidian::util::Location cellLocation, unsigned numberOfPins, bool fixed);

protected:
    ophidian::standard_cell::StandardCells stdCells_;
    ophidian::placement::Library placementLibrary_;

    ophidian::circuit::Netlist netlist_;
    ophidian::circuit::LibraryMapping libraryMapping_;
    ophidian::floorplan::Floorplan floorplan_;
    ophidian::placement::Placement placement_;
    ophidian::placement::PlacementMapping placementMapping_;
};

class AbacusFixture : public CircuitFixture
{
public:
    AbacusFixture();
};

class MultirowAbacusFixture : public CircuitFixture
{
public:
    MultirowAbacusFixture();
};

class LegalCircuitFixture : public CircuitFixture
{
public:
    LegalCircuitFixture();
protected:
    ophidian::standard_cell::Cell cellStdCell_;
};


#endif // LEGALIZATIONFIXTURE_H
