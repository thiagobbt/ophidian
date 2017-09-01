#ifndef LEGALIZATIONFIXTURE_H
#define LEGALIZATIONFIXTURE_H

#include <ophidian/placement/PlacementMapping.h>
#include <ophidian/floorplan/Floorplan.h>
#include <ophidian/design/Design.h>

#include <string>
#include <random>

class CircuitFixture
{
public:
    CircuitFixture();

    ophidian::circuit::Cell addCell(ophidian::standard_cell::Cell stdCell, std::string cellName, ophidian::util::Location cellLocation, unsigned numberOfPins, bool fixed);

public:
    ophidian::design::Design design_;
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

class LargerLegalCircuitFixture : public CircuitFixture
{
public:
    LargerLegalCircuitFixture();
protected:
    ophidian::standard_cell::Cell cellStdCell_;
};

class CircuitFixtureWithRandomCells : public CircuitFixture
{
public:
    CircuitFixtureWithRandomCells(ophidian::util::Location chipOrigin, ophidian::util::Location chipUpperCorner, unsigned numberOfCells);
};



#endif // LEGALIZATIONFIXTURE_H
