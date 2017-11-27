#include <catch.hpp>
#include <sys/time.h>

#include <ophidian/design/DesignBuilder.h>

#include <ophidian/legalization/FenceRegionIsolation.h>
#include <ophidian/legalization/CheckPerturbation.h>

void runCheckPerturbationForOneCircuit(std::string circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def");

    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();
    ophidian::legalization::Subrows subrows(design.netlist(), design.floorplan(), design.placement(), design.placementMapping());

    for (auto fence : design.fences().range()) {
        subrows.createSubrows(design.fences().area(fence));
        std::vector<ophidian::circuit::Cell> cells (design.fences().members(fence).begin(), design.fences().members(fence).end());
        ophidian::legalization::CheckPerturbation checkPerturbation(design, subrows, cells);
        int maxPerturbation = 0;
        for (auto subrow : subrows.range(ophidian::legalization::Subrow())) {
            maxPerturbation = std::max(maxPerturbation, checkPerturbation.numberOfPerturbedLines(subrow));
        }

        std::cout << "fence max perturbation " << maxPerturbation << std::endl;
    }

    ophidian::legalization::FenceRegionIsolation fenceRegionIsolation(design);
    fenceRegionIsolation.isolateAllFenceCells();

    ophidian::geometry::Box chipArea(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    subrows.createSubrows(legalizationArea);

    std::vector<ophidian::circuit::Cell> cells(design.netlist().begin(ophidian::circuit::Cell()), design.netlist().end(ophidian::circuit::Cell()));

    ophidian::legalization::CheckPerturbation checkPerturbation(design, subrows, cells);

    int maxPerturbation = 0;
    for (auto subrow : subrows.range(ophidian::legalization::Subrow())) {
        maxPerturbation = std::max(maxPerturbation, checkPerturbation.numberOfPerturbedLines(subrow));
    }

    std::cout << "max perturbation " << maxPerturbation << std::endl;
}

TEST_CASE("run check perturbation for all 2017 contest circuits", "[iccad2017][check_perturbation]")
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
        "des_perf_b_md1",
    };

    for (auto circuitName : circuitNames)
    {
        std::cout << "running circuit: " << circuitName << std::endl;
        runCheckPerturbationForOneCircuit(circuitName);
    }
}

TEST_CASE("run check perturbation for des_perf_b_md1", "[iccad2017][check_perturbation]")
{
    runCheckPerturbationForOneCircuit("des_perf_b_md1");
}

TEST_CASE("run check perturbation for des_perf_b_md2", "[iccad2017][check_perturbation]")
{
    runCheckPerturbationForOneCircuit("des_perf_b_md2");
}

TEST_CASE("run check perturbation for edit_dist_1_md1", "[iccad2017][check_perturbation]")
{
    runCheckPerturbationForOneCircuit("edit_dist_1_md1");
}

TEST_CASE("run check perturbation for edit_dist_a_md2", "[iccad2017][check_perturbation]")
{
    runCheckPerturbationForOneCircuit("edit_dist_a_md2");
}

TEST_CASE("run check perturbation for fft_2_md2", "[iccad2017][check_perturbation]")
{
    runCheckPerturbationForOneCircuit("fft_2_md2");
}

TEST_CASE("run check perturbation for fft_a_md2", "[iccad2017][check_perturbation]")
{
    runCheckPerturbationForOneCircuit("fft_a_md2");
}

TEST_CASE("run check perturbation for fft_a_md3", "[iccad2017][check_perturbation]")
{
    runCheckPerturbationForOneCircuit("fft_a_md3");
}

TEST_CASE("run check perturbation for pci_bridge32_a_md1", "[iccad2017][check_perturbation]")
{
    runCheckPerturbationForOneCircuit("pci_bridge32_a_md1");
}
