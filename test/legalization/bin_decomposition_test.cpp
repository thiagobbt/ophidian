#include <catch.hpp>

#include "legalizationfixture.h"

#include <ophidian/legalization/BinDecomposition.h>

TEST_CASE_METHOD(LargerLegalCircuitFixture, "Bin decomposition on circuit with 5 rows and 4 sites", "[legalization][bin_decomposition]") {
    ophidian::legalization::BinDecomposition binDecomposition(design_);

    ophidian::geometry::Box chipArea(design_.floorplan().chipOrigin().toPoint(), design_.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    std::vector<ophidian::circuit::Cell> cells(design_.netlist().begin(ophidian::circuit::Cell()), design_.netlist().end(ophidian::circuit::Cell()));

    binDecomposition.decomposeCircuitInBins(legalizationArea, cells, 1);

    REQUIRE(binDecomposition.size(ophidian::legalization::Bin()) == 20);
}
