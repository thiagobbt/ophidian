#include <catch.hpp>

#include "RectilinearFenceFixture.h"

#include <ophidian/legalization/RectilinearFences.h>

TEST_CASE_METHOD(RectilinearFenceFixture, "Creating block for fence region with L shape", "[legalization][rectilinear_fences]") {
    ophidian::legalization::RectilinearFences rectilinearFences(design_);

    rectilinearFences.addBlocksToRectilinearFences();

    auto block = design_.netlist().find(ophidian::circuit::Cell(), "fence0_block0");
    auto fenceCells = design_.fences().members(mFenceRegion);

    REQUIRE(fenceCells.size() == 1);
    REQUIRE(*fenceCells.begin() == block);
}
