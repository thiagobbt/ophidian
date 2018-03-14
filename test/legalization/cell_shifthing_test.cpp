#include <catch.hpp>

#include <sys/time.h>

#include "legalizationfixture.h"

#include <ophidian/legalization/CellShifting.h>

#include <ophidian/legalization/LegalizationCheck.h>

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Cell shifting on circuit already legal", "[legalization][cell_shifting]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
    };

    ophidian::legalization::CellShifting cellShifting(design_);

    cellShifting.shiftCellsInsideRows();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Cell shifting on circuit with one overlap per row", "[legalization][cell_shifting]") {
    auto cell5Location = ophidian::util::Location(19, 20);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);
    auto cell6Location = ophidian::util::Location(29, 0);
    addCell(cellStdCell_, "cell6", cell6Location, 2, false);
    auto cell7Location = ophidian::util::Location(39, 30);
    addCell(cellStdCell_, "cell7", cell7Location, 2, false);
    auto cell8Location = ophidian::util::Location(49, 10);
    addCell(cellStdCell_, "cell8", cell8Location, 2, false);
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(30, 10),
        ophidian::util::Location(20, 20),
        ophidian::util::Location(30, 0),
        ophidian::util::Location(40, 30),
        ophidian::util::Location(40, 10),
    };

    ophidian::legalization::CellShifting cellShifting(design_);

    cellShifting.shiftCellsInsideRows();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Cell shifting on illegal circuit with multirow cell", "[legalization][cell_shifting]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 30))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    auto cell5Location = ophidian::util::Location(12, 0);
    addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(5, 20),
        ophidian::util::Location(25, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(15, 0),
    };

    ophidian::legalization::CellShifting cellShifting(design_);

    cellShifting.shiftCellsInsideRows();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE("Shifting cells for circuit with random cells", "[legalization][cell_shifting][random]") {
    ophidian::util::Location chipOrigin(0, 0);
    ophidian::util::Location chipUpperCorner(100, 100);
    unsigned numberOfCells = 10;

    CircuitFixtureWithRandomAlignedCells circuit(chipOrigin, chipUpperCorner, numberOfCells);

//    std::cout << "initial locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }

    ophidian::legalization::CellShifting cellShifting(circuit.design_);

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    cellShifting.shiftCellsInsideRows();
    gettimeofday(&endTime, NULL);
    std::cout << "runtime " << endTime.tv_sec - startTime.tv_sec << " s" << std::endl;

//    std::cout << "final locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }

    REQUIRE(ophidian::legalization::legalizationCheck(circuit.design_.floorplan(), circuit.design_.placement(), circuit.design_.placementMapping(), circuit.design_.netlist(), circuit.design_.fences()));
}
