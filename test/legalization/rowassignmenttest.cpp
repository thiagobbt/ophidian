#include <catch.hpp>
#include <sys/time.h>

#include "legalizationfixture.h"

#include <ophidian/legalization/RowAssignment.h>
#include "RowAssignmentFixture.h"

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Circuit already legal", "[legalization][row_assignment]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(0, 20),
        ophidian::util::Location(0, 0),
        ophidian::util::Location(0, 30),
        ophidian::util::Location(0, 10),
    };

    ophidian::legalization::RowAssignment rowAssignment(design_);

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    rowAssignment.assignCellsToRows(legalizationArea);
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Circuit with one overflowed row", "[legalization][row_assignment]") {
    auto cell5Location = ophidian::util::Location(0, 0);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);
    auto cell6Location = ophidian::util::Location(10, 0);
    addCell(cellStdCell_, "cell6", cell6Location, 2, false);
    auto cell7Location = ophidian::util::Location(30, 0);
    addCell(cellStdCell_, "cell7", cell7Location, 2, false);
    auto cell8Location = ophidian::util::Location(40, 0);
    addCell(cellStdCell_, "cell8", cell8Location, 2, false);
    auto cell9Location = ophidian::util::Location(0, 5);
    addCell(cellStdCell_, "cell8", cell9Location, 2, false);

    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(0, 20),
        ophidian::util::Location(0, 0),
        ophidian::util::Location(0, 30),
        ophidian::util::Location(0, 10),
        ophidian::util::Location(0, 10),
        ophidian::util::Location(0, 10),
        ophidian::util::Location(0, 10),
        ophidian::util::Location(0, 10),
        ophidian::util::Location(0, 20),
    };

    ophidian::legalization::RowAssignment rowAssignment(design_);

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    rowAssignment.assignCellsToRows(legalizationArea);
}

TEST_CASE("Assigning rows in circuit with random cells", "[legalization][row_assignment][random]") {
    ophidian::util::Location chipOrigin(0, 0);
    ophidian::util::Location chipUpperCorner(2000, 2000);
    unsigned numberOfCells = 20000;

    CircuitFixtureWithRandomCells circuit(chipOrigin, chipUpperCorner, numberOfCells);

//    std::cout << "initial locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }

    RowAssignmentFixture rowAssignment(circuit.design_);

    ophidian::geometry::Box chipArea(circuit.design_.floorplan().chipOrigin().toPoint(), circuit.design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    rowAssignment.assignCellsToRows(legalizationArea);
    gettimeofday(&endTime, NULL);
    std::cout << "runtime " << endTime.tv_sec - startTime.tv_sec << " s" << std::endl;

//    std::cout << "final locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }

    REQUIRE(rowAssignment.checkSubrowsCapacities());
}
