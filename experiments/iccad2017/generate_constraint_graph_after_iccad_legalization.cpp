#include <catch.hpp>
#include <iostream>

#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>

#include <ophidian/legalization/iccad2017Legalization.h>
#include <ophidian/legalization/ConstraintGraph.h>

void generateConstraintGraphForOneCircuit(std::string circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    ophidian::legalization::iccad2017Legalization legalization(design);

    legalization.legalize();

    auto siteWidth = design.floorplan().siteUpperRightCorner(*design.floorplan().sitesRange().begin()).toPoint().x();
    auto rowHeight = design.floorplan().siteUpperRightCorner(*design.floorplan().sitesRange().begin()).toPoint().y();


    auto fence = design.fences().find("er3");
    auto fenceArea = design.fences().area(fence);
    std::vector<ophidian::circuit::Cell> fenceCells(design.fences().members(fence).begin(), design.fences().members(fence).end());

    std::vector<ophidian::circuit::Cell> legalCells;
    std::vector<ophidian::circuit::Cell> unlegalCells;

    for (auto cell : fenceCells) {
        auto cellLocation = design.placement().cellLocation(cell);
        if ((((int)cellLocation.toPoint().x() % (int)siteWidth) == 0) && (((int)cellLocation.toPoint().y() % (int)rowHeight) == 0)) {
            legalCells.push_back(cell);
        } else {
            unlegalCells.push_back(cell);
        }
    }

    std::vector<ophidian::circuit::Cell> halfLegalCells;
    for (auto cellId = 0; cellId < legalCells.size() / 2; cellId++) {
        halfLegalCells.push_back(legalCells.at(cellId));
    }

    std::cout << "number of legal cells " << halfLegalCells.size() << std::endl;

    ophidian::legalization::ConstraintGraph<ophidian::legalization::LeftComparator> horizontalConstraintGraph(design);
    horizontalConstraintGraph.buildConstraintGraph(halfLegalCells, ophidian::util::micrometer_t(fenceArea[1].min_corner().x()), ophidian::util::micrometer_t(fenceArea[1].max_corner().x()));
    horizontalConstraintGraph.removeTransitiveEdges();

    boost::geometry::index::rtree<std::pair<ophidian::geometry::Box, ophidian::circuit::Cell>, boost::geometry::index::rstar<16> > rtree;

    for (auto cell : halfLegalCells) {
        auto cellBox = design.placementMapping().geometry(cell)[0];
        rtree.insert(std::make_pair(cellBox, cell));
    }

    unsigned cellIndex = 0;
    for (auto cell : unlegalCells) {
        auto cellLocation = design.placement().cellLocation(cell).toPoint();
        cellLocation.y(fenceArea[1].min_corner().y());

        auto cellBox = design.placementMapping().geometry(cell)[0];
        auto cellWidth = cellBox.max_corner().x() - cellBox.min_corner().x();

        std::cout << "cell " << cellIndex++ << std::endl;
        std::cout << "cell width " << cellWidth << std::endl;

        bool found = false;
        double slack = 0;
        while (!found && cellLocation.y() < fenceArea[1].max_corner().y()) {
            std::vector<std::pair<ophidian::geometry::Box, ophidian::circuit::Cell>> closeNodes;
            rtree.query(boost::geometry::index::nearest(cellLocation, 1), std::back_inserter(closeNodes));
            auto closeCell = closeNodes.front().second;
            slack = horizontalConstraintGraph.slack(closeCell);
//            std::cout << "cell location " << cellLocation.x() << ", " << cellLocation.y() << std::endl;
//            std::cout << "slack " << slack << std::endl;
            found = slack >= cellWidth;
            cellLocation.y(cellLocation.y() + rowHeight);
        }

        std::cout << "found slack " << slack << std::endl;
    }

    std::cout << "worst slack " << horizontalConstraintGraph.worstSlack() << std::endl;
}

TEST_CASE("generate constraint graph for all 2017 contest circuits", "[iccad2017][ilp_legalization]")
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
        generateConstraintGraphForOneCircuit(circuitName);
    }
}

TEST_CASE("generate constraint graph for des_perf_b_md1", "[iccad2017][ilp_legalization]")
{
    generateConstraintGraphForOneCircuit("des_perf_b_md1");
}

TEST_CASE("generate constraint graph for des_perf_b_md2", "[iccad2017][ilp_legalization]")
{
    generateConstraintGraphForOneCircuit("des_perf_b_md2");
}

TEST_CASE("generate constraint graph for edit_dist_1_md1", "[iccad2017][ilp_legalization]")
{
    generateConstraintGraphForOneCircuit("edit_dist_1_md1");
}

TEST_CASE("generate constraint graph for edit_dist_a_md2", "[iccad2017][ilp_legalization]")
{
    generateConstraintGraphForOneCircuit("edit_dist_a_md2");
}

TEST_CASE("generate constraint graph for fft_2_md2", "[iccad2017][ilp_legalization]")
{
    generateConstraintGraphForOneCircuit("fft_2_md2");
}

TEST_CASE("generate constraint graph for fft_a_md2", "[iccad2017][ilp_legalization]")
{
    generateConstraintGraphForOneCircuit("fft_a_md2");
}

TEST_CASE("generate constraint graph for fft_a_md3", "[iccad2017][ilp_legalization]")
{
    generateConstraintGraphForOneCircuit("fft_a_md3");
}

TEST_CASE("generate constraint graph for pci_bridge32_a_md1", "[iccad2017][ilp_legalization]")
{
    generateConstraintGraphForOneCircuit("pci_bridge32_a_md1");
}
