#include <catch.hpp>
#include <iostream>

#include <ophidian/design/DesignBuilder.h>
#include <sys/time.h>

#include <ophidian/legalization/ConstraintGraph.h>

void generateConstraintGraphForFenceRegion(std::string circuitName, std::string fenceRegionName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./" + circuitName + "_fence_" + fenceRegionName + ".def");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();

    ophidian::legalization::ConstraintGraph<ophidian::legalization::LeftComparator> horizontalConstraintGraph(design);
    ophidian::legalization::ConstraintGraph<ophidian::legalization::BelowComparator> verticalConstraintGraph(design);

    auto fence = design.fences().find(fenceRegionName);
    auto area = design.fences().area(fence);
    std::vector<ophidian::circuit::Cell> cells(design.fences().members(fence).begin(), design.fences().members(fence).end());

    std::vector<ophidian::circuit::Cell> halfCells;
    halfCells.reserve(cells.size() / 2);
    for (auto cellId = 0; cellId < 50; cellId++) {
        halfCells.push_back(cells[cellId]);
    }

    std::cout << "origin " << area[1].min_corner().x() << ", " << area[1].min_corner().y() << std::endl;
    std::cout << "upper corner " << area[1].max_corner().x() << ", " << area[1].max_corner().y() << std::endl;

    horizontalConstraintGraph.buildConstraintGraph(halfCells, ophidian::util::micrometer_t(area[1].min_corner().x()), ophidian::util::micrometer_t(area[1].max_corner().x()));
    verticalConstraintGraph.buildConstraintGraph(halfCells, ophidian::util::micrometer_t(area[1].min_corner().y()), ophidian::util::micrometer_t(area[1].max_corner().y()));

//    horizontalConstraintGraph.exportGraph("hgraph.gv");
//    verticalConstraintGraph.exportGraph("vgraph.gv");

    unsigned iteration_index = 0;
    std::cout << "graph 1 worst slack " << horizontalConstraintGraph.worstSlack() << std::endl;
    std::cout << "graph 2 worst slack " << verticalConstraintGraph.worstSlack() << std::endl;
    while (!horizontalConstraintGraph.isFeasible() && iteration_index < 100) {
        horizontalConstraintGraph.adjustGraph(verticalConstraintGraph, ophidian::util::micrometer_t(area[1].min_corner().x()), ophidian::util::micrometer_t(area[1].max_corner().x()),
                ophidian::util::micrometer_t(area[1].min_corner().y()), ophidian::util::micrometer_t(area[1].max_corner().y()));
        std::cout << "graph 1 worst slack " << horizontalConstraintGraph.worstSlack() << std::endl;
        std::cout << "graph 2 worst slack " << verticalConstraintGraph.worstSlack() << std::endl;
        iteration_index++;
    }

}

TEST_CASE("Generate constraint graph for fence region er3 of des_perf_b_md1", "[constraint_graph][des_perf_b_md1]") {
    generateConstraintGraphForFenceRegion("des_perf_b_md1", "er3");
}
