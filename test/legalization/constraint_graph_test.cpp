#include <catch.hpp>

#include <ophidian/legalization/ConstraintGraph.h>

#include "legalizationfixture.h"

TEST_CASE_METHOD(ConstraintGraphCircuitFixture, "Constraint graph for circuit with 4 cells", "[legalization][constraint_graph]") {
    ophidian::legalization::ConstraintGraph constraintGraph(design_);

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));
    constraintGraph.buildConstraintGraph(cells);

    auto cell1 = design_.netlist().find(ophidian::circuit::Cell(), "cell1");
    auto cell2 = design_.netlist().find(ophidian::circuit::Cell(), "cell2");
    auto cell3 = design_.netlist().find(ophidian::circuit::Cell(), "cell3");
    auto cell4 = design_.netlist().find(ophidian::circuit::Cell(), "cell4");

    SECTION("Constraints of cell1") {
        REQUIRE(!constraintGraph.leftRelationship(cell1, cell2));
        REQUIRE(!constraintGraph.rightRelationship(cell1, cell2));
        REQUIRE(constraintGraph.topRelationship(cell1, cell2));
        REQUIRE(!constraintGraph.bottomRelationship(cell1, cell2));

        REQUIRE(constraintGraph.leftRelationship(cell1, cell3));
        REQUIRE(!constraintGraph.rightRelationship(cell1, cell3));
        REQUIRE(!constraintGraph.topRelationship(cell1, cell3));
        REQUIRE(!constraintGraph.bottomRelationship(cell1, cell3));

        REQUIRE(!constraintGraph.leftRelationship(cell1, cell4));
        REQUIRE(!constraintGraph.rightRelationship(cell1, cell4));
        REQUIRE(constraintGraph.topRelationship(cell1, cell4));
        REQUIRE(!constraintGraph.bottomRelationship(cell1, cell4));
    }

    SECTION("Constraints of cell2") {
        REQUIRE(!constraintGraph.leftRelationship(cell2, cell1));
        REQUIRE(!constraintGraph.rightRelationship(cell2, cell1));
        REQUIRE(!constraintGraph.topRelationship(cell2, cell1));
        REQUIRE(constraintGraph.bottomRelationship(cell2, cell1));

        REQUIRE(!constraintGraph.leftRelationship(cell2, cell3));
        REQUIRE(!constraintGraph.rightRelationship(cell2, cell3));
        REQUIRE(!constraintGraph.topRelationship(cell2, cell3));
        REQUIRE(constraintGraph.bottomRelationship(cell2, cell3));

        REQUIRE(constraintGraph.leftRelationship(cell2, cell4));
        REQUIRE(!constraintGraph.rightRelationship(cell2, cell4));
        REQUIRE(!constraintGraph.topRelationship(cell2, cell4));
        REQUIRE(!constraintGraph.bottomRelationship(cell2, cell4));
    }

    SECTION("Constraints of cell3") {
        REQUIRE(!constraintGraph.leftRelationship(cell3, cell1));
        REQUIRE(constraintGraph.rightRelationship(cell3, cell1));
        REQUIRE(!constraintGraph.topRelationship(cell3, cell1));
        REQUIRE(!constraintGraph.bottomRelationship(cell3, cell1));

        REQUIRE(!constraintGraph.leftRelationship(cell3, cell2));
        REQUIRE(!constraintGraph.rightRelationship(cell3, cell2));
        REQUIRE(constraintGraph.topRelationship(cell3, cell2));
        REQUIRE(!constraintGraph.bottomRelationship(cell3, cell2));

        REQUIRE(!constraintGraph.leftRelationship(cell3, cell4));
        REQUIRE(!constraintGraph.rightRelationship(cell3, cell4));
        REQUIRE(constraintGraph.topRelationship(cell3, cell4));
        REQUIRE(!constraintGraph.bottomRelationship(cell3, cell4));
    }

    SECTION("Constraints of cell4") {
        REQUIRE(!constraintGraph.leftRelationship(cell4, cell1));
        REQUIRE(!constraintGraph.rightRelationship(cell4, cell1));
        REQUIRE(!constraintGraph.topRelationship(cell4, cell1));
        REQUIRE(constraintGraph.bottomRelationship(cell4, cell1));

        REQUIRE(!constraintGraph.leftRelationship(cell4, cell2));
        REQUIRE(constraintGraph.rightRelationship(cell4, cell2));
        REQUIRE(!constraintGraph.topRelationship(cell4, cell2));
        REQUIRE(!constraintGraph.bottomRelationship(cell4, cell2));

        REQUIRE(!constraintGraph.leftRelationship(cell4, cell3));
        REQUIRE(!constraintGraph.rightRelationship(cell4, cell3));
        REQUIRE(!constraintGraph.topRelationship(cell4, cell3));
        REQUIRE(constraintGraph.bottomRelationship(cell4, cell3));
    }
}

TEST_CASE_METHOD(ConstraintGraphCircuitFixture, "Constraint graph for circuit with transitive edge", "[legalization][constraint_graph]") {
    ophidian::legalization::ConstraintGraph constraintGraph(design_);

    auto cell1 = design_.netlist().find(ophidian::circuit::Cell(), "cell1");
    design_.placement().placeCell(cell1, ophidian::util::Location(15, 0));
    auto cell2 = design_.netlist().find(ophidian::circuit::Cell(), "cell2");
    auto cell3 = design_.netlist().find(ophidian::circuit::Cell(), "cell3");
    auto cell4 = design_.netlist().find(ophidian::circuit::Cell(), "cell4");

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));
    constraintGraph.buildConstraintGraph(cells);

    SECTION("Constraints of cell1") {
        REQUIRE(constraintGraph.leftRelationship(cell1, cell2));
        REQUIRE(!constraintGraph.rightRelationship(cell1, cell2));
        REQUIRE(!constraintGraph.topRelationship(cell1, cell2));
        REQUIRE(!constraintGraph.bottomRelationship(cell1, cell2));

        REQUIRE(!constraintGraph.leftRelationship(cell1, cell3));
        REQUIRE(!constraintGraph.rightRelationship(cell1, cell3));
        REQUIRE(!constraintGraph.topRelationship(cell1, cell3));
        REQUIRE(constraintGraph.bottomRelationship(cell1, cell3));

        REQUIRE(!constraintGraph.leftRelationship(cell1, cell4));
        REQUIRE(!constraintGraph.rightRelationship(cell1, cell4));
        REQUIRE(!constraintGraph.topRelationship(cell1, cell4));
        REQUIRE(!constraintGraph.bottomRelationship(cell1, cell4));
    }

    SECTION("Constraints of cell2") {
        REQUIRE(!constraintGraph.leftRelationship(cell2, cell1));
        REQUIRE(constraintGraph.rightRelationship(cell2, cell1));
        REQUIRE(!constraintGraph.topRelationship(cell2, cell1));
        REQUIRE(!constraintGraph.bottomRelationship(cell2, cell1));

        REQUIRE(!constraintGraph.leftRelationship(cell2, cell3));
        REQUIRE(!constraintGraph.rightRelationship(cell2, cell3));
        REQUIRE(!constraintGraph.topRelationship(cell2, cell3));
        REQUIRE(constraintGraph.bottomRelationship(cell2, cell3));

        REQUIRE(constraintGraph.leftRelationship(cell2, cell4));
        REQUIRE(!constraintGraph.rightRelationship(cell2, cell4));
        REQUIRE(!constraintGraph.topRelationship(cell2, cell4));
        REQUIRE(!constraintGraph.bottomRelationship(cell2, cell4));
    }

    SECTION("Constraints of cell3") {
        REQUIRE(!constraintGraph.leftRelationship(cell3, cell1));
        REQUIRE(!constraintGraph.rightRelationship(cell3, cell1));
        REQUIRE(constraintGraph.topRelationship(cell3, cell1));
        REQUIRE(!constraintGraph.bottomRelationship(cell3, cell1));

        REQUIRE(!constraintGraph.leftRelationship(cell3, cell2));
        REQUIRE(!constraintGraph.rightRelationship(cell3, cell2));
        REQUIRE(constraintGraph.topRelationship(cell3, cell2));
        REQUIRE(!constraintGraph.bottomRelationship(cell3, cell2));

        REQUIRE(!constraintGraph.leftRelationship(cell3, cell4));
        REQUIRE(!constraintGraph.rightRelationship(cell3, cell4));
        REQUIRE(constraintGraph.topRelationship(cell3, cell4));
        REQUIRE(!constraintGraph.bottomRelationship(cell3, cell4));
    }

    SECTION("Constraints of cell4") {
        REQUIRE(!constraintGraph.leftRelationship(cell4, cell1));
        REQUIRE(!constraintGraph.rightRelationship(cell4, cell1));
        REQUIRE(!constraintGraph.topRelationship(cell4, cell1));
        REQUIRE(!constraintGraph.bottomRelationship(cell4, cell1));

        REQUIRE(!constraintGraph.leftRelationship(cell4, cell2));
        REQUIRE(constraintGraph.rightRelationship(cell4, cell2));
        REQUIRE(!constraintGraph.topRelationship(cell4, cell2));
        REQUIRE(!constraintGraph.bottomRelationship(cell4, cell2));

        REQUIRE(!constraintGraph.leftRelationship(cell4, cell3));
        REQUIRE(!constraintGraph.rightRelationship(cell4, cell3));
        REQUIRE(!constraintGraph.topRelationship(cell4, cell3));
        REQUIRE(constraintGraph.bottomRelationship(cell4, cell3));
    }
}
