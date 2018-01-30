#include <catch.hpp>
#include <iostream>
#include "iccad2017_wrapper.h"
#include "iccad2015_wrapper.h"
#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/legalization/iccad2017Legalization.h>
#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>
#include "MultirowAbacusFixture.h"

void showResults(std::string teamName, std::string circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder initialDesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def");

    initialDesignBuilder.build();

    ophidian::design::Design & initialDesign = initialDesignBuilder.design();


    ophidian::designBuilder::ICCAD2017ContestDesignBuilder winnerDesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/iccad2017_top3Solutions/" + teamName + "/" + circuitName + ".def");

    winnerDesignBuilder.build();

    ophidian::design::Design & winnerDesign = winnerDesignBuilder.design();

    double totalDisplacement = 0.0;
    unsigned numberOfMovableCells = 0;

    double maxDisplacement = std::numeric_limits<double>::min();

    for (auto cellIt = initialDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != initialDesign.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        auto cell = *cellIt;

        if (!initialDesign.placement().isFixed(cell))
        {
            auto initialLocation = initialDesign.placement().cellLocation(*cellIt).toPoint();

            auto cellName = initialDesign.netlist().name(cell);
            auto winnerCell = winnerDesign.netlist().find(ophidian::circuit::Cell(), cellName);

            auto currentLocation = winnerDesign.placement().cellLocation(winnerCell).toPoint();

            auto cellDisplacement = std::abs(initialLocation.x() - currentLocation.x()) +
                                    std::abs(initialLocation.y() - currentLocation.y());
            totalDisplacement += cellDisplacement;
            numberOfMovableCells++;

            maxDisplacement = std::max(maxDisplacement, cellDisplacement);
        }
    }

    double averageDisplacement = totalDisplacement / numberOfMovableCells;

    std::cout << circuitName << "," << totalDisplacement << "," << averageDisplacement << "," << maxDisplacement << std::endl;
}

TEST_CASE("cada001 results all 2017 contest circuits", "[iccad2017][winner_results]")
{
    std::cout << "running for all circuits " << std::endl;

    std::vector<std::string> circuitNames = {
        "des_perf_b_md1",
        "des_perf_b_md2",
        "edit_dist_1_md1",
        "edit_dist_a_md2",
        "fft_2_md2",
        "fft_a_md2",
        "fft_a_md3",
        "pci_bridge32_a_md1",
        "des_perf_1",
        "des_perf_a_md1",
        "des_perf_a_md2",
        "edit_dist_a_md3",
        "pci_bridge32_a_md2",
        "pci_bridge32_b_md1",
        "pci_bridge32_b_md2",
        "pci_bridge32_b_md3"
    };

    for (auto circuitName : circuitNames)
    {
        std::cout << "running circuit: " << circuitName << std::endl;
        showResults("cada001", circuitName);
    }
}

TEST_CASE("cada012 results all 2017 contest circuits", "[iccad2017][winner_results]")
{
    std::cout << "running for all circuits " << std::endl;

    std::vector<std::string> circuitNames = {
        "des_perf_b_md1",
        "des_perf_b_md2",
        "edit_dist_1_md1",
        "edit_dist_a_md2",
        "fft_2_md2",
        "fft_a_md2",
        "fft_a_md3",
        "pci_bridge32_a_md1",
        "des_perf_1",
        "des_perf_a_md1",
        "des_perf_a_md2",
        "edit_dist_a_md3",
        "pci_bridge32_a_md2",
        "pci_bridge32_b_md1",
        "pci_bridge32_b_md2",
        "pci_bridge32_b_md3"
    };

    for (auto circuitName : circuitNames)
    {
        std::cout << "running circuit: " << circuitName << std::endl;
        showResults("cada012", circuitName);
    }
}

TEST_CASE("cada041 results all 2017 contest circuits", "[iccad2017][winner_results]")
{
    std::cout << "running for all circuits " << std::endl;

    std::vector<std::string> circuitNames = {
        "des_perf_b_md1",
        "des_perf_b_md2",
        "edit_dist_1_md1",
        "edit_dist_a_md2",
        "fft_2_md2",
        "fft_a_md2",
        "fft_a_md3",
        "pci_bridge32_a_md1",
//        "des_perf_1",
        "des_perf_a_md1",
        "des_perf_a_md2",
        "edit_dist_a_md3",
        "pci_bridge32_a_md2",
        "pci_bridge32_b_md1",
        "pci_bridge32_b_md2",
        "pci_bridge32_b_md3"
    };

    for (auto circuitName : circuitNames)
    {
        std::cout << "running circuit: " << circuitName << std::endl;
        showResults("cada041", circuitName);
    }
}
