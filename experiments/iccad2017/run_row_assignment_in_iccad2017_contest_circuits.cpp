#include <catch.hpp>
#include <sys/time.h>

#include <ophidian/design/DesignBuilder.h>

#include <ophidian/legalization/RowAssignment.h>
#include <ophidian/legalization/MixedRowAssignment.h>

void runRowAssignmentForOneCircuit(std::string circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    ophidian::legalization::RowAssignment rowAssignment(design);
//    ophidian::legalization::MixedRowAssignment rowAssignment(design);

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    rowAssignment.assignCellsToRows();
    gettimeofday(&endTime, NULL);
    std::cout << "runtime " << endTime.tv_sec - startTime.tv_sec << " s" << std::endl;

    std::ofstream solutionFile;
    solutionFile.open (circuitName + "_row_assignment_solution");
    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); cellIt++) {
        auto cellName = design.netlist().name(*cellIt);
        auto cellLocation = design.placement().cellLocation(*cellIt);
        solutionFile << cellName << " " << cellLocation.toPoint().x() << " " << cellLocation.toPoint().y() << std::endl;
    }
    solutionFile.close();
}

TEST_CASE("run row assignment for all 2017 contest circuits", "[iccad2017][row_assignment]")
{
    std::cout << "running for all circuits " << std::endl;

    std::vector<std::string> circuitNames = {
//        "des_perf_b_md2",
//        "edit_dist_1_md1",
//        "edit_dist_a_md2",
        "fft_2_md2",
//        "fft_a_md2",
//        "fft_a_md3",
//        "pci_bridge32_a_md1",
//        "des_perf_b_md1",
    };

    for (auto circuitName : circuitNames)
    {
        std::cout << "running circuit: " << circuitName << std::endl;
        runRowAssignmentForOneCircuit(circuitName);
    }
}
