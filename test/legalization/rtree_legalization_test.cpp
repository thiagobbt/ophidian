#include <catch.hpp>

#include <ophidian/legalization/RtreeLegalization.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include "legalizationfixture.h"

TEST_CASE_METHOD(AbacusFixture, "Legalization: legalizing small circuit using RtreeLegalization", "[legalization][rtree]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(5, 30),
        ophidian::util::Location(15, 30),
        ophidian::util::Location(20, 20),
        ophidian::util::Location(40, 20),
        ophidian::util::Location(30, 0.0),
        ophidian::util::Location(30, 10),
        ophidian::util::Location(40, 0),
    };

    ophidian::legalization::RtreeLegalization legalization(design_);

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));
    legalization.legalizePlacement(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}
