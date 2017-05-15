#include <catch.hpp>

#include <ophidian/legalization/Abacus.h>
#include "legalizationfixture.h"

TEST_CASE_METHOD(AbacusFixture, "Legalization: legalizing small circuit using Abacus", "[legalization][abacus]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(0.7, 3.0),
        ophidian::util::Location(1.7, 3.0),
        ophidian::util::Location(2.5, 2.0),
        ophidian::util::Location(4.0, 3.0),
        ophidian::util::Location(3.0, 0.0),
        ophidian::util::Location(3.0, 1.0),
        ophidian::util::Location(4.0, 1.0),
    };

    ophidian::legalization::Abacus abacus(netlist_, floorplan_, placement_, placementMapping_);
    abacus.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = netlist_.begin(ophidian::circuit::Cell()); cellIt != netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(placement_.cellLocation(*cellIt));
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));
}
