#include <catch.hpp>
#include <iostream>

#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>

#include <ophidian/legalization/ConstraintGraph.h>
#include <ophidian/legalization/RectilinearFences.h>

void generateConstraintGraphForFenceRegion(std::string circuitName, std::string fenceRegionName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./" + circuitName + "_fence_" + fenceRegionName + ".def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    ophidian::legalization::ConstraintGraph<ophidian::legalization::LeftComparator> horizontalConstraintGraph(design);
    ophidian::legalization::ConstraintGraph<ophidian::legalization::BelowComparator> verticalConstraintGraph(design);

    ophidian::legalization::RectilinearFences rectilinearFences(design);
    rectilinearFences.addBlocksToRectilinearFences();

    auto fence = design.fences().find(fenceRegionName);
    auto area = design.fences().area(fence);
    std::vector<ophidian::circuit::Cell> cells(design.fences().members(fence).begin(), design.fences().members(fence).end());

    double fenceMinX = std::numeric_limits<double>::max();
    double fenceMaxX = -std::numeric_limits<double>::max();
    double fenceMinY = std::numeric_limits<double>::max();
    double fenceMaxY = -std::numeric_limits<double>::max();

    for (auto box : area) {
        fenceMinX = std::min(fenceMinX, box.min_corner().x());
        fenceMaxX = std::max(fenceMaxX, box.min_corner().x());
        fenceMinY = std::min(fenceMinY, box.min_corner().y());
        fenceMaxY = std::max(fenceMaxY, box.min_corner().y());
    }


    std::vector<ophidian::circuit::Cell> halfCells;
    halfCells.reserve(cells.size() / 2);
    for (auto cellId = 0; cellId < 50; cellId++) {
        halfCells.push_back(cells[cellId]);
    }

    std::cout << "origin " << fenceMinX << ", " << fenceMinY << std::endl;
    std::cout << "upper corner " << fenceMaxX << ", " << fenceMaxY << std::endl;

    horizontalConstraintGraph.buildConstraintGraph(halfCells, ophidian::util::micrometer_t(fenceMinX), ophidian::util::micrometer_t(fenceMaxX));
    verticalConstraintGraph.buildConstraintGraph(halfCells, ophidian::util::micrometer_t(fenceMinY), ophidian::util::micrometer_t(fenceMaxY));

//    horizontalConstraintGraph.exportGraph("hgraph.gv");
//    verticalConstraintGraph.exportGraph("vgraph.gv");

    unsigned iteration_index = 0;
    std::cout << "horizontal graph worst slack " << horizontalConstraintGraph.worstSlack() << std::endl;
    std::cout << "vertical graph worst slack " << verticalConstraintGraph.worstSlack() << std::endl;
    while (!horizontalConstraintGraph.isFeasible() && iteration_index < 100) {
        horizontalConstraintGraph.adjustGraph(verticalConstraintGraph, ophidian::util::micrometer_t(area[1].min_corner().x()), ophidian::util::micrometer_t(area[1].max_corner().x()),
                ophidian::util::micrometer_t(area[1].min_corner().y()), ophidian::util::micrometer_t(area[1].max_corner().y()));
        std::cout << "horizontal graph worst slack " << horizontalConstraintGraph.worstSlack() << std::endl;
        std::cout << "vertical graph worst slack " << verticalConstraintGraph.worstSlack() << std::endl;
        iteration_index++;
    }

}

TEST_CASE("Generate constraint graph for fence region er3 of des_perf_b_md1", "[constraint_graph][des_perf_b_md1]") {
    generateConstraintGraphForFenceRegion("des_perf_b_md1", "er3");
}
