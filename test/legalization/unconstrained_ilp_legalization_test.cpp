#include <catch.hpp>
#include <sys/time.h>

#include <ophidian/legalization/UnconstrainedILPLegalization.h>
#include <ophidian/legalization/LegalizationCheck.h>

#include "legalizationfixture.h"

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Unconstrained ILP in circuit already legal", "[legalization][UILP]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
    };

    ophidian::legalization::UnconstrainedILPLegalization legalization(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
//    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Unconstrained ILP in cells misaligned with sites", "[legalization][UILP]") {
    auto cell5Location = ophidian::util::Location(4, 30);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(0, 30),
    };

    ophidian::legalization::UnconstrainedILPLegalization legalization(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
//    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Unconstrained ILP in cells misaligned with rows", "[legalization][UILP]") {
    auto cell5Location = ophidian::util::Location(0, 28);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(0, 30),
    };

    ophidian::legalization::UnconstrainedILPLegalization legalization(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
//    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Unconstrained ILP in overlap between two cells", "[legalization][UILP]") {
    auto cell5Location = ophidian::util::Location(16, 20);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(20, 20),
    };

    ophidian::legalization::UnconstrainedILPLegalization legalization(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
//    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Unconstrained ILP in overlap with multirow cell", "[legalization][UILP]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(10, 10);
    ophidian::circuit::Cell cellMR = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 30),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(10, 10),
    };

    ophidian::legalization::UnconstrainedILPLegalization legalization(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
//    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE("Unconstrained ILP legalizing circuit with random cells", "[legalization][UILP][random]") {
    ophidian::util::Location chipOrigin(0, 0);
    ophidian::util::Location chipUpperCorner(5000, 5000);
    unsigned numberOfCells = 100;

    CircuitFixtureWithRandomCells circuit(chipOrigin, chipUpperCorner, numberOfCells);

//    std::cout << "initial locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }

    ophidian::legalization::UnconstrainedILPLegalization legalization(circuit.design_.netlist(), circuit.design_.floorplan(), circuit.design_.placement(), circuit.design_.placementMapping());

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    legalization.legalizePlacement();
    gettimeofday(&endTime, NULL);
    std::cout << "runtime " << endTime.tv_sec - startTime.tv_sec << " s" << std::endl;

//    std::cout << "final locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }

    REQUIRE(ophidian::legalization::checkBoundaries(circuit.design_.floorplan(), circuit.design_.placement(), circuit.design_.placementMapping(), circuit.design_.netlist(), circuit.design_.fences()));
    REQUIRE(ophidian::legalization::checkAlignment(circuit.design_.floorplan(), circuit.design_.placement(), circuit.design_.placementMapping(), circuit.design_.netlist()));
    REQUIRE(ophidian::legalization::checkCellOverlaps(circuit.design_.placementMapping(), circuit.design_.netlist()));
//    REQUIRE(ophidian::legalization::legalizationCheck(circuit.design_.floorplan(), circuit.design_.placement(), circuit.design_.placementMapping(), circuit.design_.netlist(), circuit.design_.fences()));
}
