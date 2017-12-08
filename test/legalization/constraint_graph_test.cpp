#include <catch.hpp>

#include <ophidian/legalization/ConstraintGraph.h>

#include "legalizationfixture.h"

TEST_CASE_METHOD(ConstraintGraphCircuitFixture, "Constraint graph for circuit with 4 cells", "[legalization][constraint_graph]") {
    ophidian::legalization::ConstraintGraph<ophidian::legalization::LeftComparator> horizontalConstraintGraph(design_);
    ophidian::legalization::ConstraintGraph<ophidian::legalization::BelowComparator> verticalConstraintGraph(design_);

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));
    horizontalConstraintGraph.buildConstraintGraph(cells);
    horizontalConstraintGraph.removeTransitiveEdges();
    verticalConstraintGraph.buildConstraintGraph(cells);
    verticalConstraintGraph.removeTransitiveEdges();

    auto cell1 = design_.netlist().find(ophidian::circuit::Cell(), "cell1");
    auto cell2 = design_.netlist().find(ophidian::circuit::Cell(), "cell2");
    auto cell3 = design_.netlist().find(ophidian::circuit::Cell(), "cell3");
    auto cell4 = design_.netlist().find(ophidian::circuit::Cell(), "cell4");

    SECTION("Constraints of cell1") {
        REQUIRE(!horizontalConstraintGraph.hasEdge(cell1, cell2));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell1, cell2));

        REQUIRE(horizontalConstraintGraph.hasEdge(cell1, cell3));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell1, cell3));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell1, cell4));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell1, cell4));
    }

    SECTION("Constraints of cell2") {
        REQUIRE(!horizontalConstraintGraph.hasEdge(cell2, cell1));
        REQUIRE(verticalConstraintGraph.hasEdge(cell2, cell1));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell2, cell3));
        REQUIRE(verticalConstraintGraph.hasEdge(cell2, cell3));

        REQUIRE(horizontalConstraintGraph.hasEdge(cell2, cell4));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell2, cell4));
    }

    SECTION("Constraints of cell3") {
        REQUIRE(!horizontalConstraintGraph.hasEdge(cell3, cell1));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell3, cell1));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell3, cell2));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell3, cell2));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell3, cell4));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell3, cell4));
    }

    SECTION("Constraints of cell4") {
        REQUIRE(!horizontalConstraintGraph.hasEdge(cell4, cell1));
        REQUIRE(verticalConstraintGraph.hasEdge(cell4, cell1));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell4, cell2));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell4, cell2));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell4, cell3));
        REQUIRE(verticalConstraintGraph.hasEdge(cell4, cell3));
    }
}

TEST_CASE_METHOD(ConstraintGraphCircuitFixture, "Constraint graph for circuit with transitive edge", "[legalization][constraint_graph]") {
    ophidian::legalization::ConstraintGraph<ophidian::legalization::LeftComparator> horizontalConstraintGraph(design_);
    ophidian::legalization::ConstraintGraph<ophidian::legalization::BelowComparator> verticalConstraintGraph(design_);

    auto cell1 = design_.netlist().find(ophidian::circuit::Cell(), "cell1");
    design_.placement().placeCell(cell1, ophidian::util::Location(15, 0));
    auto cell2 = design_.netlist().find(ophidian::circuit::Cell(), "cell2");
    auto cell3 = design_.netlist().find(ophidian::circuit::Cell(), "cell3");
    auto cell4 = design_.netlist().find(ophidian::circuit::Cell(), "cell4");

    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    horizontalConstraintGraph.buildConstraintGraph(cells);
    horizontalConstraintGraph.removeTransitiveEdges();

    verticalConstraintGraph.buildConstraintGraph(cells);
    verticalConstraintGraph.removeTransitiveEdges();

    SECTION("Constraints of cell1") {
        REQUIRE(horizontalConstraintGraph.hasEdge(cell1, cell2));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell1, cell2));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell1, cell3));
        REQUIRE(verticalConstraintGraph.hasEdge(cell1, cell3));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell1, cell4));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell1, cell4));
    }

    SECTION("Constraints of cell2") {
        REQUIRE(!horizontalConstraintGraph.hasEdge(cell2, cell1));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell2, cell1));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell2, cell3));
        REQUIRE(verticalConstraintGraph.hasEdge(cell2, cell3));

        REQUIRE(horizontalConstraintGraph.hasEdge(cell2, cell4));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell2, cell4));
    }

    SECTION("Constraints of cell3") {
        REQUIRE(!horizontalConstraintGraph.hasEdge(cell3, cell1));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell3, cell1));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell3, cell2));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell3, cell2));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell3, cell4));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell3, cell4));
    }

    SECTION("Constraints of cell4") {
        REQUIRE(!horizontalConstraintGraph.hasEdge(cell4, cell1));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell4, cell1));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell4, cell2));
        REQUIRE(!verticalConstraintGraph.hasEdge(cell4, cell2));

        REQUIRE(!horizontalConstraintGraph.hasEdge(cell4, cell3));
        REQUIRE(verticalConstraintGraph.hasEdge(cell4, cell3));
    }
}

TEST_CASE("Constraint graph of circuit with random cells", "[legalization][constraint_graph][random]") {
    ophidian::util::Location chipOrigin(0, 0);
    ophidian::util::Location chipUpperCorner(50000, 50000);
    unsigned numberOfCells = 500;

    CircuitFixtureWithRandomCells circuit(chipOrigin, chipUpperCorner, numberOfCells);

//    std::cout << "initial locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }

    std::vector<ophidian::circuit::Cell> cells(circuit.design_.netlist().begin(ophidian::circuit::Cell()), circuit.design_.netlist().end(ophidian::circuit::Cell()));

    ophidian::legalization::ConstraintGraph<ophidian::legalization::LeftComparator> horizontalConstraintGraph(circuit.design_);
    horizontalConstraintGraph.buildConstraintGraph(cells);

    ophidian::legalization::ConstraintGraph<ophidian::legalization::BelowComparator> verticalConstraintGraph(circuit.design_);
    verticalConstraintGraph.buildConstraintGraph(cells);

    auto & horizontalGraph = horizontalConstraintGraph.graph();
    std::cout << "number of edges in horizontal graph " << lemon::countArcs(horizontalGraph) << std::endl;

    auto & verticalGraph = verticalConstraintGraph.graph();
    std::cout << "number of edges in vertical graph " << lemon::countArcs(verticalGraph) << std::endl;

    unsigned numberOfEdgesBeforeReduction = lemon::countArcs(horizontalGraph) + lemon::countArcs(verticalGraph);

    horizontalConstraintGraph.removeTransitiveEdges();
    verticalConstraintGraph.removeTransitiveEdges();

    auto & horizontalGraphAfter = horizontalConstraintGraph.graph();
    std::cout << "number of edges in horizontal graph " << lemon::countArcs(horizontalGraphAfter) << std::endl;

    auto & verticalGraphAfter = verticalConstraintGraph.graph();
    std::cout << "number of edges in vertical graph " << lemon::countArcs(verticalGraphAfter) << std::endl;

    unsigned numberOfEdgesAfterReduction = lemon::countArcs(horizontalGraphAfter) + lemon::countArcs(verticalGraphAfter);

    REQUIRE(numberOfEdgesAfterReduction <= numberOfEdgesBeforeReduction);

//    std::cout << "final locations" << std::endl;
//    for (auto cellIt = circuit.design_.netlist().begin(ophidian::circuit::Cell()); cellIt != circuit.design_.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
//        auto cellLocation = circuit.design_.placement().cellLocation(*cellIt);
//        auto cellBox = circuit.design_.placementMapping().geometry(*cellIt)[0];
//        auto cellHeight = cellBox.max_corner().y() - cellBox.min_corner().y();
//        std::cout << "location " << cellLocation.x() << ", " << cellLocation.y() << " height " << cellHeight << std::endl;
//    }
}
