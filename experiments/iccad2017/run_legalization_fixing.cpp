#include <catch.hpp>
#include <iostream>

#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>

#include <ophidian/legalization/LegalizationFixing.h>

void runLegalizationFixingForOneCircuit(std::string circuitName) {
    std::ofstream csvFile;
//    csvFile.open (circuitName + "_movements.csv");

//    iccad2017_wrapper iccad("./input_files/ICCAD2017/" + circuitName, circuitName);

    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./" + circuitName + "_legalized_with_multirow_abacus.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    ophidian::legalization::LegalizationFixing legalizationFixing(design);

    auto fence = design.fences().find("er3");
    auto area = design.fences().area(fence);
    std::vector<ophidian::circuit::Cell> cells(design.fences().members(fence).begin(), design.fences().members(fence).end());

    legalizationFixing.fixLegalization(area, cells);


//    REQUIRE(ophidian::legalization::legalizationCheck(iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping, iccad.mNetlist));
//    REQUIRE(ophidian::legalization::checkAlignment(design.floorplan(), design.placement(), design.placementMapping(), design.netlist()));
//    REQUIRE(ophidian::legalization::checkBoundaries(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()));
//    REQUIRE(ophidian::legalization::checkCellOverlaps(design.placementMapping(), design.netlist()));
}

TEST_CASE("run legalization fixing for all 2017 contest circuits", "[iccad2017][legalization_fixing]")
{
    std::cout << "running for all circuits " << std::endl;

    std::vector<std::string> circuitNames = {
        "des_perf_b_md2",
        "edit_dist_1_md1",
        "edit_dist_a_md2",
        "fft_2_md2",
        "fft_a_md2",
        "fft_a_md3",
        "pci_bridge32_a_md1",
//        "des_perf_b_md1",
    };

    for (auto circuitName : circuitNames)
    {
        std::cout << "running circuit: " << circuitName << std::endl;
        runLegalizationFixingForOneCircuit(circuitName);
    }
}

TEST_CASE("run legalization fixing for des_perf_b_md1", "[iccad2017][legalization_fixing]")
{
    runLegalizationFixingForOneCircuit("des_perf_b_md1");
}

TEST_CASE("run legalization fixing for des_perf_b_md2", "[iccad2017][legalization_fixing]")
{
    runLegalizationFixingForOneCircuit("des_perf_b_md2");
}

TEST_CASE("run legalization fixing for edit_dist_1_md1", "[iccad2017][legalization_fixing]")
{
    runLegalizationFixingForOneCircuit("edit_dist_1_md1");
}

TEST_CASE("run legalization fixing for edit_dist_a_md2", "[iccad2017][legalization_fixing]")
{
    runLegalizationFixingForOneCircuit("edit_dist_a_md2");
}

TEST_CASE("run legalization fixing for fft_2_md2", "[iccad2017][legalization_fixing]")
{
    runLegalizationFixingForOneCircuit("fft_2_md2");
}

TEST_CASE("run legalization fixing for fft_a_md2", "[iccad2017][legalization_fixing]")
{
    runLegalizationFixingForOneCircuit("fft_a_md2");
}

TEST_CASE("run legalization fixing for fft_a_md3", "[iccad2017][legalization_fixing]")
{
    runLegalizationFixingForOneCircuit("fft_a_md3");
}

TEST_CASE("run legalization fixing for pci_bridge32_a_md1", "[iccad2017][legalization_fixing]")
{
    runLegalizationFixingForOneCircuit("pci_bridge32_a_md1");
}
