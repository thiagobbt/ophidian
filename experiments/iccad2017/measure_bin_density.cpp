#include "ophidian/design/DesignBuilder.h"
#include <catch.hpp>

namespace {
using RtreeNode = ophidian::geometry::Box;
using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16> >;

void measureBinDensityForOneCircuit(std::string circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/ICCAD2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/ICCAD2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/ICCAD2017/" + circuitName + "/placed.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    Rtree cellsRtree;
    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); cellIt++) {
        auto cellGeometry = design.placementMapping().geometry(*cellIt);
        for (auto cellBox : cellGeometry) {
            cellsRtree.insert(cellBox);
        }
    }

    double rowHeight = units::unit_cast<double>(design.floorplan().rowUpperRightCorner(*design.floorplan().rowsRange().begin()).y());

    unsigned numberOfRowsPerBin = 2;
    double binSize = numberOfRowsPerBin * rowHeight;
    std::vector<ophidian::geometry::Box> bins;

    auto chipUpperRightCorner = design.floorplan().chipUpperRightCorner();
    ophidian::geometry::Box chipArea(design.floorplan().chipOrigin().toPoint(), chipUpperRightCorner.toPoint());

    unsigned numberOfRowsInGrid = std::ceil(chipUpperRightCorner.toPoint().y() / binSize);
    unsigned numberOfColumnsInGrid = std::ceil(chipUpperRightCorner.toPoint().x() / binSize);

    bins.reserve(numberOfRowsInGrid * numberOfColumnsInGrid);
    for (unsigned rowIndex = 0; rowIndex < numberOfRowsInGrid; rowIndex++) {
        for (unsigned columnIndex = 0; columnIndex < numberOfColumnsInGrid; columnIndex++) {
            ophidian::geometry::Point binPosition(columnIndex * binSize, rowIndex * binSize);
            ophidian::geometry::Point binUpperCorner(binPosition.x() + binSize, binPosition.y() + binSize);
            ophidian::geometry::Box bin(binPosition, binUpperCorner);
            ophidian::geometry::Box binInsideChip;
            boost::geometry::intersection(bin, chipArea, binInsideChip);

            bins.push_back(binInsideChip);
        }
    }

    unsigned numberOfOverflowedBins = 0;
    unsigned numberOfUnderflowerBins = 0;
    for (auto bin : bins) {
        std::vector<RtreeNode> cellsInsideBin;
        cellsRtree.query(boost::geometry::index::intersects(bin), std::back_inserter(cellsInsideBin));
        auto cellArea = 0.0;
        for (auto cell : cellsInsideBin) {
            ophidian::geometry::Box intersection;
            boost::geometry::intersection(cell, bin, intersection);

            cellArea += boost::geometry::area(intersection);
        }

        auto binArea = boost::geometry::area(bin);

        auto density = cellArea / binArea;
//        std::cout << "bin density " << density << std::endl;

        if (density > 1) {
            numberOfOverflowedBins++;
        } else {
            numberOfUnderflowerBins++;
        }
    }

    std::cout << "number of overflowed bins " << numberOfOverflowedBins << std::endl;
    std::cout << "number of underflowed bins " << numberOfUnderflowerBins << std::endl;
}

TEST_CASE("measure bin density for all 2017 contest circuits", "[iccad2017][bin_density]")
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
        measureBinDensityForOneCircuit(circuitName);
    }
}

}
