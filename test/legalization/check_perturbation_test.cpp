#include <catch.hpp>

#include "legalizationfixture.h"

#include <ophidian/legalization/CheckPerturbation.h>

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Check perturbation in a row with singlerow cell", "[legalization][check_perturbation]") {
    ophidian::legalization::Subrows subrows(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    subrows.createSubrows(legalizationArea);

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::legalization::CheckPerturbation checkPerturbation(design_, subrows, cells);

    auto firstSubrow = *subrows.range(ophidian::legalization::Subrow()).begin();

    auto numberOfLines = checkPerturbation.numberOfPerturbedLines(firstSubrow);

    REQUIRE(numberOfLines == 1);
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Check perturbation in a row with multirow cell", "[legalization][check_perturbation]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(30.0, 0.0);
    ophidian::circuit::Cell cellMR = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    ophidian::legalization::Subrows subrows(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    subrows.createSubrows(legalizationArea);

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::legalization::CheckPerturbation checkPerturbation(design_, subrows, cells);

    auto firstSubrow = *subrows.range(ophidian::legalization::Subrow()).begin();

    auto numberOfLines = checkPerturbation.numberOfPerturbedLines(firstSubrow);

    REQUIRE(numberOfLines == 2);
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Check perturbation in a circuit with two multirow cells in the same row", "[legalization][check_perturbation]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(30.0, 0.0);
    ophidian::circuit::Cell cellMR1 = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    ophidian::util::Location cell6Location = ophidian::util::Location(30.0, 0.0);
    ophidian::circuit::Cell cellMR2 = addCell(cellStdCellMultirow, "cell6", cell6Location, 2, false);

    ophidian::legalization::Subrows subrows(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    subrows.createSubrows(legalizationArea);

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::legalization::CheckPerturbation checkPerturbation(design_, subrows, cells);

    auto firstSubrow = *subrows.range(ophidian::legalization::Subrow()).begin();

    auto numberOfLines = checkPerturbation.numberOfPerturbedLines(firstSubrow);

    REQUIRE(numberOfLines == 2);
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Check perturbation in a circuit with two multirow cells in adjacent rows", "[legalization][check_perturbation]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(30.0, 0.0);
    ophidian::circuit::Cell cellMR1 = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    ophidian::util::Location cell6Location = ophidian::util::Location(30.0, 10.0);
    ophidian::circuit::Cell cellMR2 = addCell(cellStdCellMultirow, "cell6", cell6Location, 2, false);

    ophidian::legalization::Subrows subrows(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    subrows.createSubrows(legalizationArea);

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::legalization::CheckPerturbation checkPerturbation(design_, subrows, cells);

    auto firstSubrow = *subrows.range(ophidian::legalization::Subrow()).begin();

    auto numberOfLines = checkPerturbation.numberOfPerturbedLines(firstSubrow);

    REQUIRE(numberOfLines == 3);
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Check perturbation in a circuit with three multirow cells in adjacent rows", "[legalization][check_perturbation]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(30.0, 0.0);
    ophidian::circuit::Cell cellMR1 = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    ophidian::util::Location cell6Location = ophidian::util::Location(30.0, 10.0);
    ophidian::circuit::Cell cellMR2 = addCell(cellStdCellMultirow, "cell6", cell6Location, 2, false);

    ophidian::util::Location cell7Location = ophidian::util::Location(30.0, 20.0);
    ophidian::circuit::Cell cellMR3 = addCell(cellStdCellMultirow, "cell7", cell7Location, 2, false);

    ophidian::legalization::Subrows subrows(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    subrows.createSubrows(legalizationArea);

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::legalization::CheckPerturbation checkPerturbation(design_, subrows, cells);

    auto firstSubrow = *subrows.range(ophidian::legalization::Subrow()).begin();

    auto numberOfLines = checkPerturbation.numberOfPerturbedLines(firstSubrow);

    REQUIRE(numberOfLines == 4);
}

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Check perturbation in a circuit with four multirow cells in adjacent rows", "[legalization][check_perturbation]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(30.0, 0.0);
    ophidian::circuit::Cell cellMR1 = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    ophidian::util::Location cell6Location = ophidian::util::Location(30.0, 10.0);
    ophidian::circuit::Cell cellMR2 = addCell(cellStdCellMultirow, "cell6", cell6Location, 2, false);

    ophidian::util::Location cell7Location = ophidian::util::Location(30.0, 20.0);
    ophidian::circuit::Cell cellMR3 = addCell(cellStdCellMultirow, "cell7", cell7Location, 2, false);

    ophidian::util::Location cell8Location = ophidian::util::Location(30.0, 30.0);
    ophidian::circuit::Cell cellMR4 = addCell(cellStdCellMultirow, "cell8", cell8Location, 2, false);

    ophidian::legalization::Subrows subrows(design_.netlist(), design_.floorplan(), design_.placement(), design_.placementMapping());

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    subrows.createSubrows(legalizationArea);

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::legalization::CheckPerturbation checkPerturbation(design_, subrows, cells);

    auto firstSubrow = *subrows.range(ophidian::legalization::Subrow()).begin();

    auto numberOfLines = checkPerturbation.numberOfPerturbedLines(firstSubrow);

    REQUIRE(numberOfLines == 4);
}
