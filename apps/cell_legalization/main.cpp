#include <cstdlib>
#include <iostream>

#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/legalization/iccad2017Legalization.h>
#include <ophidian/legalization/CellLegalizer.h>
#include <ophidian/legalization/FenceRegionIsolation.h>
#include <ophidian/legalization/RectilinearFences.h>
#include <ophidian/design/DesignBuilder.h>

#define DEBUG(x) []() { std::cerr << #x << ": " << x << std::endl; }()
#define CHECK(x) [&]()->bool { if (!x) { std::cerr << "CHECK FAILED: " << #x << std::endl; return false; } else { return true; } }()
#define CHECKDESIGN(design) [&]()->bool { \
    return CHECK(ophidian::legalization::checkAlignment(design.floorplan(), design.placement(), design.placementMapping(), design.netlist())) and \
    CHECK(ophidian::legalization::checkBoundaries(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences())) and \
    CHECK(ophidian::legalization::checkCellOverlaps(design.placementMapping(), design.netlist())); \
}()

typedef ophidian::geometry::Point point;
typedef boost::geometry::model::box<point> Box;

bool optimizeCircuit(const std::string & circuitName) {
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder originalCircuitBuilder(std::string(getenv("HOME")) + "/benchmarks/ICCAD2017/" + circuitName + "/cells_modified.lef",
                                                                                  std::string(getenv("HOME")) + "/benchmarks/ICCAD2017/" + circuitName + "/tech.lef",
                                                                                  std::string(getenv("HOME")) + "/benchmarks/ICCAD2017/" + circuitName + "/placed.def",
                                                                                  std::string(getenv("HOME")) + "/benchmarks/ICCAD2017/" + circuitName + "/placement.constraints");
    originalCircuitBuilder.build();
    ophidian::design::Design & originalDesign = originalCircuitBuilder.design();

    ophidian::designBuilder::ICCAD2017ContestDesignBuilder circuitBuilder(std::string(getenv("HOME")) + "/benchmarks/ICCAD2017/" + circuitName + "/cells_modified.lef",
                                                                          std::string(getenv("HOME")) + "/benchmarks/ICCAD2017/" + circuitName + "/tech.lef",
                                                                          std::string(getenv("HOME")) + "/benchmarks/ICCAD2017/_legalized/" + circuitName + "_legalized_with_multirow_abacus.def",
                                                                          std::string(getenv("HOME")) + "/benchmarks/ICCAD2017/" + circuitName + "/placement.constraints");
    circuitBuilder.build();
    ophidian::design::Design & design = circuitBuilder.design();

    CHECKDESIGN(design);
}

int main(int argc, char const *argv[])
{
    std::vector<std::string> circuitNames = {
        // "des_perf_1",
        // "des_perf_a_md1",
        // "des_perf_a_md2",
        // "des_perf_b_md1",
        // "des_perf_b_md2",
        // "edit_dist_1_md1",
        // "edit_dist_a_md2",
        // "edit_dist_a_md3",
        "fft_2_md2",
        "fft_a_md2",
        "fft_a_md3",
        "pci_bridge32_a_md1",
        "pci_bridge32_a_md2",
        "pci_bridge32_b_md1",
        "pci_bridge32_b_md2",
        "pci_bridge32_b_md3",
    };

    for (auto circuitName : circuitNames) {
        std::cout << "optimizing circuit: " << circuitName << std::endl;
        optimizeCircuit(circuitName);
    }
}
