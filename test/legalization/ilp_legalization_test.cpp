#include <catch.hpp>

#include <ophidian/legalization/ILPLegalization.h>
#include <ophidian/legalization/LegalizationCheck.h>

#include "legalizationfixture.h"

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Circuit already legal", "[legalization][ILP]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
    };

    ophidian::legalization::ILPLegalization legalization(netlist_, floorplan_, placement_, placementMapping_);
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = netlist_.begin(ophidian::circuit::Cell()); cellIt != netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(placement_.cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_, fences_));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Cells misaligned with sites", "[legalization][ILP]") {
    auto cell5Location = ophidian::util::Location(4, 30);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(0, 30),
    };

    ophidian::legalization::ILPLegalization legalization(netlist_, floorplan_, placement_, placementMapping_);
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = netlist_.begin(ophidian::circuit::Cell()); cellIt != netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(placement_.cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_, fences_));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Cells misaligned with rows", "[legalization][ILP]") {
    auto cell5Location = ophidian::util::Location(0, 28);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(0, 30),
    };

    ophidian::legalization::ILPLegalization legalization(netlist_, floorplan_, placement_, placementMapping_);
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = netlist_.begin(ophidian::circuit::Cell()); cellIt != netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(placement_.cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_, fences_));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Overlap between two cells", "[legalization][ILP]") {
    auto cell5Location = ophidian::util::Location(16, 20);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(20, 20),
    };

    ophidian::legalization::ILPLegalization legalization(netlist_, floorplan_, placement_, placementMapping_);
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = netlist_.begin(ophidian::circuit::Cell()); cellIt != netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(placement_.cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_, fences_));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Overlap with multirow cell", "[legalization][ILP]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    placementLibrary_.geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(10, 10);
    ophidian::circuit::Cell cellMR = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(0, 10),
    };

    ophidian::legalization::ILPLegalization legalization(netlist_, floorplan_, placement_, placementMapping_);
    legalization.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = netlist_.begin(ophidian::circuit::Cell()); cellIt != netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(placement_.cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_, fences_));
}

TEST_CASE("Legalizing circuit with random cells", "[legalization][ILP][random]") {
    ophidian::util::Location chipOrigin(0.0, 0.0);
    ophidian::util::Location chipUpperCorner(2500.0, 2500.0);
    unsigned numberOfCells = 1000;

    CircuitFixtureWithRandomCells circuitFixture(chipOrigin, chipUpperCorner, numberOfCells);

//    std::cout << "locations before legalization" << std::endl;
//    for (auto cellIt = circuitFixture.netlist_.begin(ophidian::circuit::Cell()); cellIt != circuitFixture.netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
//        std::cout << circuitFixture.placement_.cellLocation(*cellIt).x() << ", " << circuitFixture.placement_.cellLocation(*cellIt).y() << std::endl;
//    }

    ophidian::legalization::ILPLegalization legalization(circuitFixture.netlist_, circuitFixture.floorplan_, circuitFixture.placement_, circuitFixture.placementMapping_);
    legalization.legalizePlacement();

//    std::cout << "locations after legalization" << std::endl;
//    for (auto cellIt = circuitFixture.netlist_.begin(ophidian::circuit::Cell()); cellIt != circuitFixture.netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
//        std::cout << circuitFixture.placement_.cellLocation(*cellIt).x() << ", " << circuitFixture.placement_.cellLocation(*cellIt).y() << std::endl;
//    }

//    REQUIRE(ophidian::legalization::legalizationCheck(circuitFixture.floorplan_, circuitFixture.placement_, circuitFixture.placementMapping_, circuitFixture.netlist_));
    REQUIRE(ophidian::legalization::checkBoundaries(circuitFixture.floorplan_, circuitFixture.placementMapping_, circuitFixture.netlist_));
    REQUIRE(ophidian::legalization::checkAlignment(circuitFixture.floorplan_, circuitFixture.placement_, circuitFixture.placementMapping_, circuitFixture.netlist_));
    REQUIRE(ophidian::legalization::checkCellOverlaps(circuitFixture.placementMapping_, circuitFixture.netlist_));
}
