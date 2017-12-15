#include <catch.hpp>
#include <iostream>

#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>

#include <ophidian/legalization/ILPLegalizationWithConstraintGraph.h>

void runILPLeaglizationWithConstraintGraphForOneCircuit(std::string circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
//                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def");
                                                                                  "./des_perf_b_md1_fence_er3.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    ophidian::legalization::ILPLegalizationWithConstraintGraph ilpLegalization(design);

    auto fence = design.fences().find("er3");
    auto area = design.fences().area(fence);
    std::vector<ophidian::circuit::Cell> cells(design.fences().members(fence).begin(), design.fences().members(fence).end());

    std::vector<ophidian::circuit::Cell> halfCells;
    halfCells.reserve(cells.size() / 2);
    for (auto cellId = 0; cellId < 50; cellId++) {
        halfCells.push_back(cells[cellId]);
    }

    std::cout << "legalizing " << std::endl;
    ilpLegalization.legalize(halfCells, area[1]);

////    design.writeDefFile("out.def", halfCells);
}

TEST_CASE("run ILP legalization with constraint graph for all 2017 contest circuits", "[iccad2017][ilp_legalization]")
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
        runILPLeaglizationWithConstraintGraphForOneCircuit(circuitName);
    }
}

TEST_CASE("run ILP legalization with constraint graph for des_perf_b_md1", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationWithConstraintGraphForOneCircuit("des_perf_b_md1");
}

TEST_CASE("run ILP legalization with constraint graph for des_perf_b_md2", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationWithConstraintGraphForOneCircuit("des_perf_b_md2");
}

TEST_CASE("run ILP legalization with constraint graph for edit_dist_1_md1", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationWithConstraintGraphForOneCircuit("edit_dist_1_md1");
}

TEST_CASE("run ILP legalization with constraint graph for edit_dist_a_md2", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationWithConstraintGraphForOneCircuit("edit_dist_a_md2");
}

TEST_CASE("run ILP legalization with constraint graph for fft_2_md2", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationWithConstraintGraphForOneCircuit("fft_2_md2");
}

TEST_CASE("run ILP legalization with constraint graph for fft_a_md2", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationWithConstraintGraphForOneCircuit("fft_a_md2");
}

TEST_CASE("run ILP legalization with constraint graph for fft_a_md3", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationWithConstraintGraphForOneCircuit("fft_a_md3");
}

TEST_CASE("run ILP legalization with constraint graph for pci_bridge32_a_md1", "[iccad2017][ilp_legalization]")
{
    runILPLeaglizationWithConstraintGraphForOneCircuit("pci_bridge32_a_md1");
}
