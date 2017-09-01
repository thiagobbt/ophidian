#include <catch.hpp>
#include <iostream>
#include <ophidian/design/Design.h>
#include <ophidian/design/DesignBuilder.h>

void measureAreaRelationshipForOneCircuit(std::string circuitName) {
    std::cout << "circuit " << circuitName << std::endl;

    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/ICCAD2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/ICCAD2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/ICCAD2017/" + circuitName + "/placed.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    std::cout << "area of fences" << std::endl;
    for (auto fence : design.fences().range()) {
        auto fenceRegion = design.fences().area(fence);
        double fenceArea = 0.0;
        for (auto box : fenceRegion) {
            fenceArea += boost::geometry::area(box);
        }
        double cellArea = 0.0;
        for (auto cell : design.fences().members(fence)) {
            auto cellGeometry = design.placementMapping().geometry(cell);
            for (auto box : cellGeometry) {
                cellArea += boost::geometry::area(box);
            }
        }

        std::cout << "fence area " << design.fences().name(fence) << " " << fenceArea << std::endl;
        std::cout << "cells area " << cellArea << std::endl;
    }

    double totalCellArea = 0.0;
    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        auto cellGeometry = design.placementMapping().geometry(*cellIt);

        for (auto box : cellGeometry) {
            totalCellArea += boost::geometry::area(box);
        }
    }

    auto chipUpperRightCorner = design.floorplan().chipUpperRightCorner();
    double chipArea = units::unit_cast<double>(chipUpperRightCorner.x()) * units::unit_cast<double>(chipUpperRightCorner.y());

    std::cout << "chip area " << chipArea << std::endl;
    std::cout << "cells area " << totalCellArea << std::endl;

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
