#include <catch.hpp>
#include <sys/time.h>

#include <ophidian/design/DesignBuilder.h>

#include <ophidian/legalization/RowAssignment.h>

void runRowAssignmentForOneCircuit(std::string circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/ICCAD2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/ICCAD2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/ICCAD2017/" + circuitName + "/placed.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    ophidian::legalization::RowAssignment rowAssignment(design);

    ophidian::geometry::Box chipArea(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    rowAssignment.assignCellsToRows(legalizationArea);
    gettimeofday(&endTime, NULL);
    std::cout << "runtime " << endTime.tv_sec - startTime.tv_sec << " s" << std::endl;
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
