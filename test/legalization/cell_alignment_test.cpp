#include <catch.hpp>

#include "legalizationfixture.h"

#include <ophidian/legalization/CellAlignment.h>

TEST_CASE_METHOD(MisalignedCircuitFixture, "Aligning four cells with different misalignments", "[legalization][cell_alignment]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
    };

    ophidian::legalization::CellAlignment cellAlignment(design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));
}
