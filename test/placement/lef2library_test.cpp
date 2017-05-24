#include "lef2library_test.h"
#include <catch.hpp>

#include <ophidian/placement/Lef2Library.h>

using namespace ophidian;

class Lef2LibraryFixture{
public:
    Lef2LibraryFixture():lef(parser.readFile("./input_files/simple.lef")){
        library.reset(new placement::Library(stdCells));
    }
    parser::LefParser parser;
    std::unique_ptr<parser::Lef> lef;
    std::unique_ptr<placement::Library> library;
    standard_cell::StandardCells stdCells;
};

TEST_CASE_METHOD(Lef2LibraryFixture, "Lef2Library: Test library cell geometries.", "[standard_cell][library][placement][lef][dbunits]")
{
    placement::lef2Library(*lef, *library, stdCells);
    REQUIRE(library->geometry(stdCells.find(standard_cell::Cell(), "INV_X1")) == util::MultiBox({geometry::Box(geometry::Point(0.0*lef->databaseUnits(), 0.0*lef->databaseUnits()),
                                                                                                               geometry::Point(0.760*lef->databaseUnits(), 1.71*lef->databaseUnits()))}));
}

TEST_CASE_METHOD(Lef2LibraryFixture, "Lef2Library: Test library pin offset.", "[standard_cell][library][placement][lef][dbunits]")
{
    placement::lef2Library(*lef, *library, stdCells);
    REQUIRE(library->pinOffset(stdCells.find(standard_cell::Pin(), "INV_X1:a")) == util::Location(0.5*(0.210+0.340)*lef->databaseUnits(), 0.5*(0.34+0.405)*lef->databaseUnits()));
}
