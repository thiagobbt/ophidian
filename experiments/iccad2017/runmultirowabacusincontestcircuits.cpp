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

void runMultirowAbacusForOneCircuit2015(std::string circuitName) {
    iccad2015_wrapper iccad("./input_files/benchmarks/" + circuitName, circuitName);

    unsigned movableCells = 0;
    unsigned fixedCells = 0;

    ophidian::util::micrometer_t siteWidth(200);
    ophidian::util::micrometer_t rowHeight(2000);
    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::util::Location> initialLocations(iccad.mNetlist.makeProperty<ophidian::util::Location>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, bool> initialFixed(iccad.mNetlist.makeProperty<bool>(ophidian::circuit::Cell()));
    for (auto cellIt = iccad.mNetlist.begin(ophidian::circuit::Cell()); cellIt != iccad.mNetlist.end(ophidian::circuit::Cell()); ++cellIt)
    {
        if (iccad.mPlacement.isFixed(*cellIt))
        {
            fixedCells++;
        }
        else {
            movableCells++;
        }

        initialLocations[*cellIt] = iccad.mPlacement.cellLocation(*cellIt);
        initialFixed[*cellIt] = iccad.mPlacement.isFixed(*cellIt);

        //alinhamento de celulas
        auto cellLocation = iccad.mPlacement.cellLocation(*cellIt);
        auto alignedX = std::floor(units::unit_cast<double>(cellLocation.x() / siteWidth)) * siteWidth;
        auto alignedY = std::floor(units::unit_cast<double>(cellLocation.y() / rowHeight)) * rowHeight;
        iccad.mPlacement.placeCell(*cellIt, ophidian::util::Location(alignedX, alignedY));
    }

    std::cout << "fixed cells " << fixedCells << std::endl;
    std::cout << "movable cells " << movableCells << std::endl;

    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    std::vector<ophidian::circuit::Cell> cells (iccad.mNetlist.begin(ophidian::circuit::Cell()), iccad.mNetlist.end(ophidian::circuit::Cell()));
    ophidian::geometry::Box chipArea(iccad.mFloorplan.chipOrigin().toPoint(), iccad.mFloorplan.chipUpperRightCorner().toPoint());
    ophidian::util::MultiBox legalizationArea({chipArea});
    MultiRowAbacusFixture multirowAbacus(iccad.mNetlist, iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping);
    multirowAbacus.legalizePlacement(cells, legalizationArea);


    struct timeval endTime;
    gettimeofday(&endTime, NULL);

    double runtime = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec)/1000000.0;

    for (auto cellIt = iccad.mNetlist.begin(ophidian::circuit::Cell()); cellIt != iccad.mNetlist.end(ophidian::circuit::Cell()); ++cellIt)
    {
        iccad.mPlacement.fixLocation(*cellIt, initialFixed[*cellIt]);
    }

//    REQUIRE(ophidian::legalization::legalizationCheck(iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping, iccad.mNetlist));
    REQUIRE(ophidian::legalization::checkAlignment(iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping, iccad.mNetlist));
    REQUIRE(ophidian::legalization::checkBoundaries(iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping, iccad.mNetlist, iccad.mFences));
    REQUIRE(ophidian::legalization::checkCellOverlaps(iccad.mPlacementMapping, iccad.mNetlist));

    ophidian::util::micrometer_t totalDisplacement;
    unsigned numberOfMovableCells = 0;
    for (auto cellIt = iccad.mNetlist.begin(ophidian::circuit::Cell()); cellIt != iccad.mNetlist.end(ophidian::circuit::Cell()); ++cellIt)
    {
        if (!initialFixed[*cellIt])
        {
            auto currentLocation = iccad.mPlacement.cellLocation(*cellIt);
            auto cellDisplacement = std::abs(units::unit_cast<double>(initialLocations[*cellIt].x() - currentLocation.x())) +
                                    std::abs(units::unit_cast<double>(initialLocations[*cellIt].y() - currentLocation.y()));
            totalDisplacement = totalDisplacement + ophidian::util::micrometer_t(cellDisplacement);
            numberOfMovableCells++;
        }
    }
    ophidian::util::micrometer_t averageDisplacement = totalDisplacement / numberOfMovableCells;

    std::cout << circuitName << "," << totalDisplacement << "," << averageDisplacement << "," << runtime << std::endl;

    multirowAbacus.writeCsvWithCellsPerSubrow(circuitName + "_cells_per_subrow.csv");

//    iccad.writeDefFile(circuitName + "_legalized.def");
}

void runMultirowAbacusForOneCircuit(std::string circuitName) {
    std::ofstream csvFile;
    csvFile.open (circuitName + "_movements.csv");

//    iccad2017_wrapper iccad("./input_files/ICCAD2017/" + circuitName, circuitName);

    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def");
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

//    REQUIRE(ophidian::legalization::legalizationCheck(iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping, iccad.mNetlist));
    REQUIRE(ophidian::legalization::checkAlignment(design.floorplan(), design.placement(), design.placementMapping(), design.netlist()));
    REQUIRE(ophidian::legalization::checkBoundaries(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()));
    REQUIRE(ophidian::legalization::checkCellOverlaps(design.placementMapping(), design.netlist()));

    ophidian::util::micrometer_t totalDisplacement;
    unsigned numberOfMovableCells = 0;

    double maxDisplacement = std::numeric_limits<double>::min();

    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        if (!design.placement().isFixed(*cellIt))
        {
            auto currentLocation = design.placement().cellLocation(*cellIt);
            auto cellDisplacement = std::abs(units::unit_cast<double>(initialLocations[*cellIt].x() - currentLocation.x())) +
                                    std::abs(units::unit_cast<double>(initialLocations[*cellIt].y() - currentLocation.y()));
            totalDisplacement = totalDisplacement + ophidian::util::micrometer_t(cellDisplacement);
            numberOfMovableCells++;

            maxDisplacement = std::max(maxDisplacement, cellDisplacement);

            csvFile << cellDisplacement << std::endl;
        } else {
            auto currentLocation = design.placement().cellLocation(*cellIt);
            auto currentOrientation = design.placement().cellOrientation(*cellIt);
            if (currentLocation != initialLocations[*cellIt] || currentOrientation != initialOrientations[*cellIt]) {
                std::string cellName = design.netlist().name(*cellIt);
                std::cout << "cell " << cellName << " has been relocated or flipped " << std::endl;
            }
        }
    }

    std::cout << "max displacement " << maxDisplacement << std::endl;

    ophidian::util::micrometer_t averageDisplacement = totalDisplacement / numberOfMovableCells;

    std::cout << circuitName << "," << totalDisplacement << "," << averageDisplacement << "," << runtime << std::endl;

//    multirowAbacus.writeCsvWithCellsPerSubrow(circuitName + "_cells_per_subrow.csv");

    design.writeDefFile(circuitName + "_legalized.def");

    csvFile.close();
}

TEST_CASE("run multirow abacus for all 2015 contest circuits", "[iccad2015][multirow_abacus]")
{
    std::cout << "running for all circuits " << std::endl;

    std::vector<std::string> circuitNames = {
        "superblue18",
//        "superblue4",
//        "superblue16",
//        "superblue3",
//        "superblue5",
//        "superblue7",
//        "superblue1",
//        "superblue10",
    };

    for (auto circuitName : circuitNames)
    {
        runMultirowAbacusForOneCircuit2015(circuitName);
    }
}

TEST_CASE("run multirow abacus for all 2017 contest circuits", "[iccad2017][multirow_abacus]")
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
        runMultirowAbacusForOneCircuit(circuitName);
    }
}

TEST_CASE("run multirow abacus for des_perf_b_md1", "[iccad2017][multirow_abacus]")
{
    runMultirowAbacusForOneCircuit("des_perf_b_md1");
}

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
