#include <catch.hpp>
#include <iostream>

#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>

#include <ophidian/legalization/ILPLegalization.h>

void runILPLeaglizationForOneCircuit(std::string circuitName) {
    std::ofstream csvFile;
//    csvFile.open (circuitName + "_movements.csv");

//    iccad2017_wrapper iccad("./input_files/ICCAD2017/" + circuitName, circuitName);

    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    ophidian::legalization::ILPLegalization ilpLegalization(design.netlist(), design.floorplan(), design.placement(), design.placementMapping());

    auto fence = design.fences().find("er3");
    auto area = design.fences().area(fence);
    std::vector<ophidian::circuit::Cell> cells(design.fences().members(fence).begin(), design.fences().members(fence).end());

//    legalizationFixing.legalizePlacement();


//    REQUIRE(ophidian::legalization::legalizationCheck(iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping, iccad.mNetlist));
//    REQUIRE(ophidian::legalization::checkAlignment(design.floorplan(), design.placement(), design.placementMapping(), design.netlist()));
//    REQUIRE(ophidian::legalization::checkBoundaries(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()));
//    REQUIRE(ophidian::legalization::checkCellOverlaps(design.placementMapping(), design.netlist()));
}

TEST_CASE("run ILP legalization for all 2017 contest circuits", "[iccad2017][ilp_legalization]")
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
        runILPLeaglizationForOneCircuit(circuitName);
    }
}

TEST_CASE("run ILP legalization for des_perf_b_md1", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationForOneCircuit("des_perf_b_md1");
}

TEST_CASE("run ILP legalization for des_perf_b_md2", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationForOneCircuit("des_perf_b_md2");
}

TEST_CASE("run ILP legalization for edit_dist_1_md1", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationForOneCircuit("edit_dist_1_md1");
}

TEST_CASE("run ILP legalization for edit_dist_a_md2", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationForOneCircuit("edit_dist_a_md2");
}

TEST_CASE("run ILP legalization for fft_2_md2", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationForOneCircuit("fft_2_md2");
}

TEST_CASE("run ILP legalization for fft_a_md2", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationForOneCircuit("fft_a_md2");
}

TEST_CASE("run ILP legalization for fft_a_md3", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationForOneCircuit("fft_a_md3");
}

TEST_CASE("run ILP legalization for pci_bridge32_a_md1", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationForOneCircuit("pci_bridge32_a_md1");
}
