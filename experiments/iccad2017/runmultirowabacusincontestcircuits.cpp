#include <catch.hpp>
#include <iostream>
#include <cmath>
#include <boost/accumulators/accumulators.hpp>
#include <boost/bind.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include "iccad2017_wrapper.h"
#include "iccad2015_wrapper.h"
#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/legalization/iccad2017Legalization.h>
#include <ophidian/legalization/CellLegalizer.h>
#include <ophidian/legalization/FenceRegionIsolation.h>
#include <ophidian/legalization/RectilinearFences.h>
#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>
#include "MultirowAbacusFixture.h"

#define DEBUG(x) do { std::cerr << #x << ": " << x << std::endl; } while (0)

void runMultirowAbacusForOneCircuit(std::string circuitName) {
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

    std::map<std::string, int64_t> displacementMap;
    std::vector<ophidian::circuit::Cell> cellsByDisplacement(design.netlist().begin(ophidian::circuit::Cell()), design.netlist().end(ophidian::circuit::Cell()));

    int64_t totalDisplacementAfterLegalization = 0;

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

    boost::accumulators::accumulator_set<int64_t, boost::accumulators::stats<boost::accumulators::tag::variance, boost::accumulators::tag::mean> > acc;
    for (auto displacement : displacementMap) {
        acc(std::get<1>(displacement));
    }

    int64_t stdDevDisplacement = sqrt(boost::accumulators::variance(acc));

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

    // cellLegalizer.buildRtree(cellsByDisplacement);
    // cellLegalizer.buildRtree({design.netlist().begin(ophidian::circuit::Cell()), design.netlist().end(ophidian::circuit::Cell())});

    int maxDisplacementAllowed = boost::accumulators::mean(acc) + (stdDevDisplacement * 2);
    std::cout << "Max displacement allowed: " << maxDisplacementAllowed << std::endl;

    int fail_count = 0;
    int count = 0;
    int fence_num = 0;

    ophidian::legalization::RectilinearFences rectilinearFences(design);
    rectilinearFences.addBlocksToRectilinearFences();

    for (auto fence : design.fences().range()) {
        std::cout << "Processing fence " << fence_num++ << std::endl;

        auto fenceArea = design.fences().area(fence);
        ophidian::geometry::Box fenceBoundingBox;
        boost::geometry::envelope(fenceArea.toMultiPolygon(), fenceBoundingBox);

        auto cellsInFence = design.fences().members(fence);
        cellLegalizer.buildRtree({cellsInFence.begin(), cellsInFence.end()});

        for (auto cell : design.fences().members(fence)) {
            if (displacementMap[design.netlist().name(cell)] <= maxDisplacementAllowed) break;
            bool result = cellLegalizer.legalizeCell(cell, initialLocations[cell].toPoint(), fenceBoundingBox);
            count++;
            if (!result) fail_count++;
            if (count > 0 && count % 100 == 0) std::cout << count << " cells processed (" << fail_count << " failed)" << std::endl;
        }
    }

    rectilinearFences.eraseBlocks();

    ophidian::legalization::FenceRegionIsolation fenceRegionIsolation(design);
    fenceRegionIsolation.isolateAllFenceCells();

    std::vector<ophidian::circuit::Cell> nonFenceCellsByDisplacement;
    std::copy_if(cellsByDisplacement.begin(), cellsByDisplacement.end(), std::back_inserter(nonFenceCellsByDisplacement),
        [&](ophidian::circuit::Cell c) {
            return !design.placement().cellHasFence(c);
        }
    );

    fenceRegionIsolation.restoreAllFenceCells();

    auto circuitBoundingBox = boost::geometry::model::box<ophidian::geometry::Point>(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint());

    cellLegalizer.buildRtree(nonFenceCellsByDisplacement);

    // for (auto cellIt = cellsByDisplacement.begin(); cellIt < lastCellToRelegalize; cellIt++) {
    for (auto cell : nonFenceCellsByDisplacement) {
        if (displacementMap[design.netlist().name(cell)] <= maxDisplacementAllowed) break;
        bool result = cellLegalizer.legalizeCell(cell, initialLocations[cell].toPoint(), circuitBoundingBox);
        count++;
        if (!result) fail_count++;
        if (count > 0 && count % 100 == 0) std::cout << count << " cells processed (" << fail_count << " failed, displacement: " << displacementMap[design.netlist().name(cell)] <<  ")" << std::endl;
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

// TEST_CASE("run multirow abacus for edit_dist_1_md1", "[iccad2017][multirow_abacus]")
// {
//     runMultirowAbacusForOneCircuit("edit_dist_1_md1");
// }

// TEST_CASE("run multirow abacus for edit_dist_a_md2", "[iccad2017][multirow_abacus]")
// {
//     runMultirowAbacusForOneCircuit("edit_dist_a_md2");
// }

// TEST_CASE("run multirow abacus for fft_2_md2", "[iccad2017][multirow_abacus]")
// {
//     runMultirowAbacusForOneCircuit("fft_2_md2");
// }

// TEST_CASE("run multirow abacus for fft_a_md2", "[iccad2017][multirow_abacus]")
// {
//     runMultirowAbacusForOneCircuit("fft_a_md2");
// }

// TEST_CASE("run multirow abacus for fft_a_md3", "[iccad2017][multirow_abacus]")
// {
//     runMultirowAbacusForOneCircuit("fft_a_md3");
// }

// TEST_CASE("run multirow abacus for pci_bridge32_a_md1", "[iccad2017][multirow_abacus]")
// {
//     runMultirowAbacusForOneCircuit("pci_bridge32_a_md1");
// }
