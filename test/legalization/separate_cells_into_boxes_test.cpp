#include <catch.hpp>

#include "legalizationfixture.h"

#include <ophidian/legalization/SeparateCellsIntoBoxes.h>

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Separate circuit with 4 cells into one box", "[legalization][separate_cells_into_boxes]") {
    ophidian::legalization::SeparateCellsIntoBoxes separateCells(design_);

    ophidian::geometry::Box box1(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox area({box1});
    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    separateCells.separateCells(cells, area);

    auto region0 = separateCells.find("region_0");

    auto cell1 = design_.netlist().find(ophidian::circuit::Cell(), "cell1");
    auto cell2 = design_.netlist().find(ophidian::circuit::Cell(), "cell2");
    auto cell3 = design_.netlist().find(ophidian::circuit::Cell(), "cell3");
    auto cell4 = design_.netlist().find(ophidian::circuit::Cell(), "cell4");

    REQUIRE(separateCells.cellRegion(cell1) == region0);
    REQUIRE(separateCells.cellRegion(cell2) == region0);
    REQUIRE(separateCells.cellRegion(cell3) == region0);
    REQUIRE(separateCells.cellRegion(cell4) == region0);
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Separate circuit with 4 cells into two boxes", "[legalization][separate_cells_into_boxes]") {
    ophidian::legalization::SeparateCellsIntoBoxes separateCells(design_);

    ophidian::geometry::Box box1(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(26, 40));
    ophidian::geometry::Box box2(ophidian::geometry::Point(26, 0), ophidian::geometry::Point(50, 40));
    ophidian::util::MultiBox area({box1, box2});
    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    separateCells.separateCells(cells, area);

    auto region0 = separateCells.find("region_0");
    auto region1 = separateCells.find("region_1");

    auto cell1 = design_.netlist().find(ophidian::circuit::Cell(), "cell1");
    auto cell2 = design_.netlist().find(ophidian::circuit::Cell(), "cell2");
    auto cell3 = design_.netlist().find(ophidian::circuit::Cell(), "cell3");
    auto cell4 = design_.netlist().find(ophidian::circuit::Cell(), "cell4");

    REQUIRE(separateCells.cellRegion(cell1) == region0);
    REQUIRE(separateCells.cellRegion(cell2) == region0);
    REQUIRE(separateCells.cellRegion(cell3) == region1);
    REQUIRE(separateCells.cellRegion(cell4) == region1);
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Separate circuit with 4 cells into four boxes", "[legalization][separate_cells_into_boxes]") {
    ophidian::legalization::SeparateCellsIntoBoxes separateCells(design_);

    ophidian::geometry::Box box1(ophidian::geometry::Point(20, 30), ophidian::geometry::Point(30, 40));
    ophidian::geometry::Box box2(ophidian::geometry::Point(40, 20), ophidian::geometry::Point(50, 30));
    ophidian::geometry::Box box3(ophidian::geometry::Point(20, 0), ophidian::geometry::Point(30, 10));
    ophidian::geometry::Box box4(ophidian::geometry::Point(40, 10), ophidian::geometry::Point(50, 20));
    ophidian::util::MultiBox area({box1, box2, box3, box4});
    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    separateCells.separateCells(cells, area);

    auto region0 = separateCells.find("region_0");
    auto region1 = separateCells.find("region_1");
    auto region2 = separateCells.find("region_2");
    auto region3 = separateCells.find("region_3");

    auto cell1 = design_.netlist().find(ophidian::circuit::Cell(), "cell1");
    auto cell2 = design_.netlist().find(ophidian::circuit::Cell(), "cell2");
    auto cell3 = design_.netlist().find(ophidian::circuit::Cell(), "cell3");
    auto cell4 = design_.netlist().find(ophidian::circuit::Cell(), "cell4");

    REQUIRE(separateCells.cellRegion(cell1) == region0);
    REQUIRE(separateCells.cellRegion(cell2) == region2);
    REQUIRE(separateCells.cellRegion(cell3) == region1);
    REQUIRE(separateCells.cellRegion(cell4) == region3);
}
