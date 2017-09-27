#include <catch.hpp>
#include <sys/time.h>

#include <ophidian/design/DesignBuilder.h>

#include <ophidian/legalization/GreedyRowAssignment.h>
#include <ophidian/legalization/CellShifting.h>
#include <ophidian/legalization/LegalizationCheck.h>

void readRowAssignmentFile(std::string circuitName, ophidian::design::Design & design) {
    std::fstream solutionFile;
    solutionFile.open(circuitName + "_row_assignment_solution", std::fstream::in);

    if (!solutionFile.good()) {
        assert(false);
    }

    for (unsigned cellIndex = 0; cellIndex < design.netlist().size(ophidian::circuit::Cell()); cellIndex++) {
        std::string cellName;
        solutionFile >> cellName;
        double cellX, cellY;
        solutionFile >> cellX;
        solutionFile >> cellY;

        auto cell = design.netlist().find(ophidian::circuit::Cell(), cellName);
        ophidian::util::Location cellLocation(cellX, cellY);
        design.placement().placeCell(cell, cellLocation);
    }
}

void runCellShiftingForOneCircuit(std::string circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();
    ophidian::legalization::CellShifting cellShifting(design);

//    ophidian::legalization::GreedyRowAssignment rowAssignment(design);
////    ophidian::legalization::MixedRowAssignment rowAssignment(design);
//    rowAssignment.assignCellsToRows();
    readRowAssignmentFile(circuitName, design);

    REQUIRE(ophidian::legalization::checkBoundaries(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()));
    REQUIRE(ophidian::legalization::checkAlignment(design.floorplan(), design.placement(), design.placementMapping(), design.netlist()));

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);
    cellShifting.shiftCellsInsideRows();
    gettimeofday(&endTime, NULL);
    std::cout << "runtime " << endTime.tv_sec - startTime.tv_sec << " s" << std::endl;

    design.writeDefFile(circuitName + "_legalized.def");

    REQUIRE(ophidian::legalization::legalizationCheck(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()));
}

TEST_CASE("run cell shifting for all 2017 contest circuits", "[iccad2017][cell_shifting]")
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
        runCellShiftingForOneCircuit(circuitName);
    }
}
