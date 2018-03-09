#include <catch.hpp>
#include <iostream>
#include <cmath>
#include "iccad2017_wrapper.h"
#include "iccad2015_wrapper.h"
#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/legalization/iccad2017Legalization.h>
#include <ophidian/legalization/CellLegalizer.h>
#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>
#include "MultirowAbacusFixture.h"

void runMultirowAbacusForOneCircuit(std::string circuitName) {
    std::ofstream csvFile;
//    csvFile.open (circuitName + "_movements.csv");

//    iccad2017_wrapper iccad("./input_files/ICCAD2017/" + circuitName, circuitName);

    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placement.constraints");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    unsigned movableCells = 0;
    unsigned fixedCells = 0;

    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::util::Location> initialLocations(design.netlist().makeProperty<ophidian::util::Location>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, std::string> initialOrientations(design.netlist().makeProperty<std::string>(ophidian::circuit::Cell()));

    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        if (design.placement().isFixed(*cellIt))
        {
            fixedCells++;
        }
        else {
            movableCells++;
        }
        initialLocations[*cellIt] = design.placement().cellLocation(*cellIt);
        initialOrientations[*cellIt] = design.placement().cellOrientation(*cellIt);
    }

    std::cout << "fixed cells " << fixedCells << std::endl;
    std::cout << "movable cells " << movableCells << std::endl;

    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    ophidian::legalization::iccad2017Legalization iccad2017(design);
    iccad2017.legalize();

    struct timeval endTime;
    gettimeofday(&endTime, NULL);

    double runtime = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec)/1000000.0;

    std::map<std::string, int> displacementMap;
    std::vector<ophidian::circuit::Cell> cellsByDisplacement(design.netlist().begin(ophidian::circuit::Cell()), design.netlist().end(ophidian::circuit::Cell()));

    int totalDisplacementAfterLegalization = 0;

    std::ofstream displacementFile1;
    displacementFile1.open("./" + circuitName + "_displacement_after_legalization.csv");
    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        if (!design.placement().isFixed(*cellIt))
        {
            auto currentLocation = design.placement().cellLocation(*cellIt);
            auto cellDisplacement = std::abs(units::unit_cast<double>(initialLocations[*cellIt].x() - currentLocation.x())) +
                                    std::abs(units::unit_cast<double>(initialLocations[*cellIt].y() - currentLocation.y()));
            totalDisplacementAfterLegalization += cellDisplacement;
            displacementMap[design.netlist().name(*cellIt)] = cellDisplacement;
            displacementFile1 << ophidian::util::micrometer_t(cellDisplacement) << std::endl;
        } else {
            displacementMap[design.netlist().name(*cellIt)] = 0;
        }
    }
    displacementFile1.close();

    int numCells = design.netlist().size(ophidian::circuit::Cell());
    int meanDisplacement = totalDisplacementAfterLegalization / numCells;
    int sumOfSquaredDifferences = 0;

    for (auto displacement : displacementMap) {
        sumOfSquaredDifferences += pow(std::get<1>(displacement) - meanDisplacement, 2);
    }

    int halfStdDevDisplacement = sqrt(sumOfSquaredDifferences / (numCells-1))/2;

    auto displacementComp = [&](const ophidian::circuit::Cell& a, const ophidian::circuit::Cell& b) {
        return displacementMap[design.netlist().name(a)] > displacementMap[design.netlist().name(b)];
    };

    std::sort(cellsByDisplacement.begin(), cellsByDisplacement.end(), displacementComp);

    std::cout << "Max displacement: " << displacementMap[design.netlist().name(cellsByDisplacement[0])] << std::endl;

    // REQUIRE(ophidian::legalization::legalizationCheck(iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping, iccad.mNetlist));
    REQUIRE(ophidian::legalization::checkAlignment(design.floorplan(), design.placement(), design.placementMapping(), design.netlist()));
    REQUIRE(ophidian::legalization::checkBoundaries(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()));
    REQUIRE(ophidian::legalization::checkCellOverlaps(design.placementMapping(), design.netlist()));

    design.writeDefFile(circuitName + "_legalized_with_multirow_abacus.def");

    std::cout << std::boolalpha;

    ophidian::legalization::CellLegalizer cellLegalizer(design);


    // int numOfCellsToRelegalize = design.netlist().size(ophidian::circuit::Cell()) / 100;

    // std::cout << "numOfCellsToRelegalize: " << numOfCellsToRelegalize << std::endl;

    // auto lastCellToRelegalize = cellsByDisplacement.begin();
    // std::advance(lastCellToRelegalize, numOfCellsToRelegalize);

    std::vector<ophidian::circuit::Cell> legalized_cells(cellsByDisplacement);
    cellLegalizer.buildTree(legalized_cells);

    int maxDisplacementAllowed = meanDisplacement + (halfStdDevDisplacement * 2);

    int fail_count = 0;
    int count = 0;
    // for (auto cellIt = cellsByDisplacement.begin(); cellIt < lastCellToRelegalize; cellIt++) {
    for (auto cellIt = cellsByDisplacement.begin(); cellIt < cellsByDisplacement.end(); cellIt++) {
        if (displacementMap[design.netlist().name(*cellIt)] <= maxDisplacementAllowed) break;
        legalized_cells.erase(legalized_cells.begin());
        bool result = cellLegalizer.legalizeCell(*cellIt, initialLocations[*cellIt].toPoint(), boost::geometry::model::box<ophidian::geometry::Point>(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint()));
        legalized_cells.push_back(*cellIt);
        count++;
        if (!result) fail_count++;
        if (count > 0 && count % 100 == 0) std::cout << count << " cells processed (" << fail_count << " failed)" << std::endl;
    }

    ophidian::util::micrometer_t totalDisplacement(0);
    unsigned numberOfMovableCells = 0;

    double maxDisplacement = std::numeric_limits<double>::min();

    std::ofstream displacementFile;
    displacementFile.open("./" + circuitName + "_displacement_after_cell_legalization.csv");

    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        if (!design.placement().isFixed(*cellIt))
        {
            auto currentLocation = design.placement().cellLocation(*cellIt);
            auto cellDisplacement = std::abs(units::unit_cast<double>(initialLocations[*cellIt].x() - currentLocation.x())) +
                                    std::abs(units::unit_cast<double>(initialLocations[*cellIt].y() - currentLocation.y()));
            totalDisplacement = totalDisplacement + ophidian::util::micrometer_t(cellDisplacement);
            displacementFile << ophidian::util::micrometer_t(cellDisplacement) << std::endl;
            numberOfMovableCells++;

            maxDisplacement = std::max(maxDisplacement, cellDisplacement);

//            csvFile << cellDisplacement << std::endl;
        } else {
            auto currentLocation = design.placement().cellLocation(*cellIt);
            auto currentOrientation = design.placement().cellOrientation(*cellIt);
            if (currentLocation != initialLocations[*cellIt] || currentOrientation != initialOrientations[*cellIt]) {
                std::string cellName = design.netlist().name(*cellIt);
                std::cout << "cell " << cellName << " has been relocated or flipped " << std::endl;
            }
        }
    }

    displacementFile.close();

    std::cout << "max displacement " << maxDisplacement << std::endl;

    ophidian::util::micrometer_t averageDisplacement = totalDisplacement / numberOfMovableCells;

    std::cout << circuitName << "," << totalDisplacement << "," << averageDisplacement << "," << runtime << std::endl;

//    multirowAbacus.writeCsvWithCellsPerSubrow(circuitName + "_cells_per_subrow.csv");

    design.writeDefFile(circuitName + "_legalized_with_multirow_abacus_after_cell_relegalization.def");

    REQUIRE(ophidian::legalization::checkAlignment(design.floorplan(), design.placement(), design.placementMapping(), design.netlist()));
    REQUIRE(ophidian::legalization::checkBoundaries(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()));
    REQUIRE(ophidian::legalization::checkCellOverlaps(design.placementMapping(), design.netlist()));

//    csvFile.close();
}

// TEST_CASE("run multirow abacus for all 2017 contest circuits", "[iccad2017][multirow_abacus]")
// {
//     std::cout << "running for all circuits " << std::endl;

//     std::vector<std::string> circuitNames = {
//         "des_perf_b_md2",
//         "edit_dist_1_md1",
//         "edit_dist_a_md2",
//         "fft_2_md2",
//         "fft_a_md2",
//         "fft_a_md3",
//         "pci_bridge32_a_md1",
// //        "des_perf_b_md1",
//     };

//     for (auto circuitName : circuitNames)
//     {
//         std::cout << "running circuit: " << circuitName << std::endl;
//         runMultirowAbacusForOneCircuit(circuitName);
//     }
// }

// TEST_CASE("run multirow abacus for des_perf_b_md1", "[iccad2017][multirow_abacus]")
// {
//     runMultirowAbacusForOneCircuit("des_perf_b_md1");
// }

TEST_CASE("run multirow abacus for des_perf_b_md2", "[iccad2017][multirow_abacus]")
{
    runMultirowAbacusForOneCircuit("des_perf_b_md2");
}

TEST_CASE("run multirow abacus for edit_dist_1_md1", "[iccad2017][multirow_abacus]")
{
    runMultirowAbacusForOneCircuit("edit_dist_1_md1");
}

TEST_CASE("run multirow abacus for edit_dist_a_md2", "[iccad2017][multirow_abacus]")
{
    runMultirowAbacusForOneCircuit("edit_dist_a_md2");
}

TEST_CASE("run multirow abacus for fft_2_md2", "[iccad2017][multirow_abacus]")
{
    runMultirowAbacusForOneCircuit("fft_2_md2");
}

TEST_CASE("run multirow abacus for fft_a_md2", "[iccad2017][multirow_abacus]")
{
    runMultirowAbacusForOneCircuit("fft_a_md2");
}

TEST_CASE("run multirow abacus for fft_a_md3", "[iccad2017][multirow_abacus]")
{
    runMultirowAbacusForOneCircuit("fft_a_md3");
}

TEST_CASE("run multirow abacus for pci_bridge32_a_md1", "[iccad2017][multirow_abacus]")
{
    runMultirowAbacusForOneCircuit("pci_bridge32_a_md1");
}
