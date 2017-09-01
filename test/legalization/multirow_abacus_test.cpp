#include <catch.hpp>

#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include "legalizationfixture.h"

TEST_CASE_METHOD(MultirowAbacusFixture, "Legalization: legalizing small circuit using MultirowAbacus", "[legalization][abacus][multirow]")
{
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(7, 30),
        ophidian::util::Location(17, 30),
        ophidian::util::Location(25, 20),
        ophidian::util::Location(40, 30),
        ophidian::util::Location(40, 0.0),
        ophidian::util::Location(30, 0.0),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(35, 20),
    };

    ophidian::legalization::MultirowAbacus multirowAbacus(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());

    std::vector<ophidian::circuit::Cell> cells (design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    multirowAbacus.legalizePlacement(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}
