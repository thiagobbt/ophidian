#include <catch.hpp>

#include <ophidian/legalization/Abacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include "legalizationfixture.h"

TEST_CASE_METHOD(AbacusFixture, "Legalization: legalizing small circuit using Abacus", "[legalization][abacus]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(7, 30),
        ophidian::util::Location(17, 30),
        ophidian::util::Location(20, 20),
        ophidian::util::Location(40, 30),
        ophidian::util::Location(30, 0.0),
        ophidian::util::Location(30, 10),
        ophidian::util::Location(40, 10),
    };

    ophidian::legalization::Abacus abacus(netlist_, floorplan_, placement_, placementMapping_);
    abacus.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = netlist_.begin(ophidian::circuit::Cell()); cellIt != netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(placement_.cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_, fences_));
}
