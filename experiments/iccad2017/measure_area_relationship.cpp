#include <catch.hpp>
#include <iostream>
#include "iccad2017_wrapper.h"

void measureAreaRelationshipForOneCircuit(std::string circuitName) {
    std::cout << "circuit " << circuitName << std::endl;

    iccad2017_wrapper iccad("./input_files/ICCAD2017/" + circuitName, circuitName);

    double totalCellArea = 0.0;
    for (auto cellIt = iccad.mNetlist.begin(ophidian::circuit::Cell()); cellIt != iccad.mNetlist.end(ophidian::circuit::Cell()); ++cellIt)
    {
        auto cellGeometry = iccad.mPlacementMapping.geometry(*cellIt);

        auto cellWidth = cellGeometry[0].max_corner().x() - cellGeometry[0].min_corner().x();
        auto cellHeight = cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y();

        auto cellArea = cellWidth * cellHeight;
        totalCellArea += cellArea;
    }

    auto chipUpperRightCorner = iccad.mFloorplan.chipUpperRightCorner();
    double chipArea = units::unit_cast<double>(chipUpperRightCorner.x()) * units::unit_cast<double>(chipUpperRightCorner.y());

    std::cout << "area relationship " << chipArea / totalCellArea << std::endl;
}

TEST_CASE("measure area relationship for all 2017 contest circuits", "[iccad2017][area]")
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
        measureAreaRelationshipForOneCircuit(circuitName);
    }
}
