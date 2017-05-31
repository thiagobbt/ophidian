#include <catch.hpp>

#include <ophidian/legalization/LegalizationCheck.h>
#include "legalizationfixture.h"

TEST_CASE_METHOD(LegalCircuitFixture, "Legalization Check: legal circuit", "[legalization][check]") {
    bool aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(aligned);
    bool boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    bool overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(overlaps);

    bool legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(legalized);
}

TEST_CASE_METHOD(LegalCircuitFixture, "Legalization Check: overlapping cells in the same location", "[legalization][check]") {
    ophidian::standard_cell::Cell cellStdCellMultirow = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 2))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    placementLibrary_.geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(2.0, 0.0);
    ophidian::circuit::Cell cellMR = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    bool aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(aligned);
    bool boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    bool overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(!overlaps);

    bool legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);
}


TEST_CASE_METHOD(LegalCircuitFixture, "Legalization Check: misaligned", "[legalization][check]") {
    auto cell5Location = ophidian::util::Location(0.5, 3.0);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    bool aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!aligned);
    bool boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    bool overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(overlaps);

    bool legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);
}

TEST_CASE_METHOD(LegalCircuitFixture, "Legalization Check: outside boundaries", "[legalization][check]") {
    auto cell5Location = ophidian::util::Location(5.0, 3.0);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    bool aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(aligned);
    bool boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(!boundaries);
    bool overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(overlaps);

    bool legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);

    auto cell6Location = ophidian::util::Location(4.5, 2.0);
    addCell(cellStdCell_, "cell6", cell6Location, 2, false);

    aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!aligned);
    boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(!boundaries);
    overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(overlaps);

    legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);

    auto cell7Location = ophidian::util::Location(2.5, -0.5);
    addCell(cellStdCell_, "cell7", cell7Location, 2, false);

    aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!aligned);
    boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(!boundaries);
    overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(!overlaps);

    legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);
}

TEST_CASE_METHOD(LegalCircuitFixture, "Legalization Check: overlaped", "[legalization][check]") {
    auto cell5Location = ophidian::util::Location(3.0, 3.0);
    addCell(cellStdCell_, "cell5", cell5Location, 2, false);

    bool aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(aligned);
    bool boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    bool overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(!overlaps);

    bool legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);

    auto cell6Location = ophidian::util::Location(1.5, 1.5);
    addCell(cellStdCell_, "cell6", cell6Location, 2, false);

    aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!aligned);
    boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(!overlaps);

    legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);

    auto cell7Location = ophidian::util::Location(4.5, 1.5);
    addCell(cellStdCell_, "cell7", cell7Location, 2, false);

    aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!aligned);
    boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(!boundaries);
    overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(!overlaps);

    legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);
}

TEST_CASE_METHOD(LegalCircuitFixture, "Legalization Check: cell multirow", "[legalization][check]") {
    bool aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(aligned);
    bool boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    bool overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(overlaps);
    bool legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(legalized);

    ophidian::standard_cell::Cell cellStdCellMultirow = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 2))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    placementLibrary_.geometry(cellStdCellMultirow, stdCellGeometryMR);

    ophidian::util::Location cell5Location = ophidian::util::Location(3.0, 1.0);
    ophidian::circuit::Cell cellMR = addCell(cellStdCellMultirow, "cell5", cell5Location, 2, false);

    aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(aligned);
    boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(overlaps);
    legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(legalized);

    placement_.placeCell(cellMR, ophidian::util::Location(2.5, 1.0));
    aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!aligned);
    boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(overlaps);
    legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);

    placement_.placeCell(cellMR, ophidian::util::Location(3.0, 2.0));
    aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(aligned);
    boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(boundaries);
    overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(!overlaps);
    legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);

    placement_.placeCell(cellMR, ophidian::util::Location(2.0, 3.0));
    aligned = ophidian::legalization::checkAlignment(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(aligned);
    boundaries = ophidian::legalization::checkBoundaries(floorplan_, placementMapping_, netlist_);
    REQUIRE(!boundaries);
    overlaps = ophidian::legalization::checkCellOverlaps(placementMapping_, netlist_);
    REQUIRE(overlaps);
    legalized = ophidian::legalization::legalizationCheck(floorplan_, placement_, placementMapping_, netlist_);
    REQUIRE(!legalized);
}
