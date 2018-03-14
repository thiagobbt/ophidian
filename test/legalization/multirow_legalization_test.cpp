#include <catch.hpp>
#include <sys/time.h>

#include <ophidian/legalization/MultirowLegalization.h>
#include <ophidian/legalization/CellAlignment.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/legalization/CellByCellLegalization.h>

#include "legalizationfixture.h"

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Multirow legalization: circuit already legal", "[legalization][multirow_legalization]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
    };

    ophidian::legalization::CellAlignment cellAlignment(design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::circuit::Cell> cells (design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    ophidian::legalization::CellByCellLegalization<ophidian::legalization::MultirowLegalization> legalization(design_);
    legalization.legalize(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Multirow legalization: cells misaligned with sites", "[legalization][multirow_legalization]") {
    auto cell5Location = ophidian::util::Location(2, 30);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(0, 30),
    };

    ophidian::legalization::CellAlignment cellAlignment(design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::circuit::Cell> cells (design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    ophidian::legalization::CellByCellLegalization<ophidian::legalization::MultirowLegalization> legalization(design_);
    legalization.legalize(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Multirow legalization: cells misaligned with rows", "[legalization][multirow_legalization]") {
    auto cell5Location = ophidian::util::Location(0, 28);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(0, 30),
    };

    ophidian::legalization::CellAlignment cellAlignment(design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::circuit::Cell> cells (design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    ophidian::legalization::CellByCellLegalization<ophidian::legalization::MultirowLegalization> legalization(design_);
    legalization.legalize(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
//        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Multirow legalization: overlap between two cells", "[legalization][multirow_legalization]") {
    auto cell5Location = ophidian::util::Location(16, 20);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(20, 20),
    };

    ophidian::legalization::CellAlignment cellAlignment(design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::circuit::Cell> cells (design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    ophidian::legalization::CellByCellLegalization<ophidian::legalization::MultirowLegalization> legalization(design_);
    legalization.legalize(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Multirow legalization: overlap between two cells with further cluster overlap", "[legalization][multirow_legalization]") {
    auto cell5Location = ophidian::util::Location(16, 20);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    auto cell6Location = ophidian::util::Location(0, 20);
    addCell(cellStdCell_, "cell6", cell6Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(20, 20),
        ophidian::util::Location(0, 20),
    };

    ophidian::legalization::CellAlignment cellAlignment(design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::circuit::Cell> cells (design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    ophidian::legalization::CellByCellLegalization<ophidian::legalization::MultirowLegalization> legalization(design_);
    legalization.legalize(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Multirow legalization: overlap between two cells exceeding right boundary", "[legalization][multirow_legalization]") {
    auto cell5Location = ophidian::util::Location(35, 10);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(30, 10),
    };

    ophidian::legalization::CellAlignment cellAlignment(design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::circuit::Cell> cells (design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    ophidian::legalization::CellByCellLegalization<ophidian::legalization::MultirowLegalization> legalization(design_);
    legalization.legalize(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Multirow legalization: overlap with multirow cell", "[legalization][multirow_legalization]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(10, 10);
    ophidian::circuit::Cell cellMR = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(10, 20),
        ophidian::util::Location(20, 0),
        ophidian::util::Location(30, 30),
        ophidian::util::Location(40, 10),
        ophidian::util::Location(10, 0),
    };

    ophidian::legalization::CellAlignment cellAlignment(design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::circuit::Cell> cells (design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    ophidian::legalization::CellByCellLegalization<ophidian::legalization::MultirowLegalization> legalization(design_);
    legalization.legalize(cells, legalizationArea);

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = design_.netlist().begin(ophidian::circuit::Cell()); cellIt != design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(design_.placement().cellLocation(*cellIt));
        std::cout << cellLocations.back().x() << ", " << cellLocations.back().y() << std::endl;
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));

    REQUIRE(ophidian::legalization::legalizationCheck(design_.floorplan(), design_.placement(), design_.placementMapping(), design_.netlist(), design_.fences()));
}

TEST_CASE("Multirow legalization: legalizing circuit with random cells", "[legalization][multirow_legalization][random]") {
    ophidian::util::Location chipOrigin(0, 0);
    ophidian::util::Location chipUpperCorner(50000, 50000);
    unsigned numberOfCells = 50;

    CircuitFixtureWithRandomCells circuit(chipOrigin, chipUpperCorner, numberOfCells);

//    std::cout << "initial locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }

    ophidian::legalization::CellAlignment cellAlignment(circuit.design_);
    cellAlignment.alignCellsToSitesAndRows();

    std::vector<ophidian::circuit::Cell> cells (circuit.design_.netlist().begin(ophidian::circuit::Cell()), circuit.design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::geometry::Box chipArea(circuit.design_.floorplan().chipOrigin().toPoint(), circuit.design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    ophidian::legalization::CellByCellLegalization<ophidian::legalization::MultirowLegalization> legalization(circuit.design_);

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    legalization.legalize(cells, legalizationArea);
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
