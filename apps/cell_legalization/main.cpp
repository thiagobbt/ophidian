#include <cstdlib>
#include <iostream>

#include <boost/accumulators/accumulators.hpp>
#include <boost/bind.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/legalization/iccad2017Legalization.h>
#include <ophidian/legalization/CellLegalizer.h>
#include <ophidian/legalization/CellAlignment.h>
#include <ophidian/legalization/FenceRegionIsolation.h>
#include <ophidian/legalization/RectilinearFences.h>
#include <ophidian/design/DesignBuilder.h>

#define DEBUG(x) [&]() { std::cerr << #x << ": " << x << std::endl; }()
#define CHECK(x) [&]()->bool { if (!x) { std::cerr << "CHECK FAILED: " << #x << std::endl; return false; } else { return true; } }()
#define CHECKDESIGN(design) [&]()->bool { \
    return CHECK(ophidian::legalization::checkAlignment(design.floorplan(), design.placement(), design.placementMapping(), design.netlist())) and \
    CHECK(ophidian::legalization::checkBoundaries(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences())) and \
    CHECK(ophidian::legalization::checkCellOverlaps(design.placementMapping(), design.netlist())); \
}()

typedef ophidian::geometry::Point point;
typedef boost::geometry::model::box<point> Box;

long long displacement(const ophidian::util::Location & a, const ophidian::util::Location & b) {
    return std::abs(units::unit_cast<long long>(a.x() - b.x())) +
           std::abs(units::unit_cast<long long>(a.y() - b.y()));
}

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

    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::util::Location> initialLocations(design.netlist().makeProperty<ophidian::util::Location>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, std::string> initialOrientations(design.netlist().makeProperty<std::string>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, long long> cellDisplacement(design.netlist().makeProperty<long long>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::circuit::Cell> cellMapper(design.netlist().makeProperty<ophidian::circuit::Cell>(ophidian::circuit::Cell()));

    for (auto cellIt = originalDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != originalDesign.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        auto originalCell = *cellIt;
        auto currentCell = design.netlist().find(ophidian::circuit::Cell(), originalDesign.netlist().name(originalCell));

        cellMapper[originalCell] = currentCell;

        initialLocations[currentCell] = originalDesign.placement().cellLocation(originalCell);
        initialOrientations[currentCell] = originalDesign.placement().cellOrientation(originalCell);
        cellDisplacement[currentCell] = displacement(initialLocations[currentCell], design.placement().cellLocation(currentCell));
    }

    boost::accumulators::accumulator_set<long long, boost::accumulators::features<boost::accumulators::tag::variance, boost::accumulators::tag::mean>> accBefore;
    for (auto displacement : cellDisplacement) {
        accBefore(displacement);
    }

    auto stdDevDisplacement = sqrt(boost::accumulators::variance(accBefore));
    auto meanDisplacement = boost::accumulators::mean(accBefore);
    // auto maxDisplacement = boost::accumulators::max(accBefore);
    // auto totalDisplacement = boost::accumulators::sum(accBefore);
    auto maxDisplacementAllowed = meanDisplacement + (stdDevDisplacement * 2);

    DEBUG(stdDevDisplacement);
    DEBUG(meanDisplacement);
    // DEBUG(maxDisplacement);
    // DEBUG(totalDisplacement);
    DEBUG(maxDisplacementAllowed);

    ophidian::legalization::CellLegalizer cellLegalizer(design);
    ophidian::legalization::CellAlignment cellAligner(design);

    ophidian::legalization::RectilinearFences rectilinearFences(design);
    rectilinearFences.addBlocksToRectilinearFences();

    auto findNewCellLocation = [&](ophidian::circuit::Cell & cell, ophidian::geometry::Box & area) -> ophidian::util::Location {
        auto cellLocation =  initialLocations[cell];
        auto site         = *design.floorplan().sitesRange().begin();
        auto siteWidth    =  design.floorplan().siteUpperRightCorner(site).x();
        auto rowHeight    =  design.floorplan().siteUpperRightCorner(site).y();

        auto newX = std::round(units::unit_cast<double>(cellLocation.x() / siteWidth)) * siteWidth;
        auto newY = std::round(units::unit_cast<double>(cellLocation.y() / rowHeight)) * rowHeight;

        auto cellAlignment = design.placementMapping().alignment(cell);
        auto siteHeight = design.floorplan().siteUpperRightCorner(*design.floorplan().sitesRange().begin()).y();
        auto cellPlacedInOddRow = std::fmod((newY/siteHeight), 2.0);

        if ((cellPlacedInOddRow and cellAlignment == ophidian::placement::RowAlignment::EVEN) or
            (!cellPlacedInOddRow and cellAlignment == ophidian::placement::RowAlignment::ODD)) {
            newY = newY - rowHeight;
        }

        auto stdCell = design.libraryMapping().cellStdCell(cell);
        auto cellGeometry = design.library().geometry(stdCell)[0];

        auto cellWidth = cellGeometry.max_corner().x();
        auto cellHeight = cellGeometry.max_corner().y();

        while (units::unit_cast<double>(newX) + cellWidth  > area.max_corner().x()) newX = newX - siteWidth;
        while (units::unit_cast<double>(newY) + cellHeight > area.max_corner().y()) newY = newY - (rowHeight * 2);

        while (units::unit_cast<double>(newX) < area.min_corner().x()) newX = newX + siteWidth;
        while (units::unit_cast<double>(newY) < area.min_corner().y()) newY = newY + (rowHeight * 2);

        return ophidian::util::Location(newX, newY);
    };

    int fail_count = 0;
    int count = 0;
    int fence_num = 0;

    for (auto fence : design.fences().range()) {
        // std::cout << "Processing fence " << fence_num++ << std::endl;

        auto fenceArea = design.fences().area(fence);
        ophidian::geometry::Box fenceBoundingBox;
        boost::geometry::envelope(fenceArea.toMultiPolygon(), fenceBoundingBox);

        auto cellsInFence = design.fences().members(fence);
        cellLegalizer.buildRtree({cellsInFence.begin(), cellsInFence.end()});

        for (auto cell : design.fences().members(fence)) {
            if (cellDisplacement[cell] <= maxDisplacementAllowed) continue;
            bool result = cellLegalizer.legalizeCell(cell, findNewCellLocation(cell, fenceBoundingBox).toPoint(), fenceBoundingBox);
            count++;
            if (!result) fail_count++;
            // if (count > 0 && count % 100 == 0) std::cout << count << " cells processed (" << fail_count << " failed)" << std::endl;
        }

        // std::cout << "End fence: " << count << " cells processed (" << fail_count << " failed)" << std::endl;
        // count = 0;
        // fail_count = 0;
    }

    rectilinearFences.eraseBlocks();
    if (!CHECKDESIGN(design)) {
        std::cout << "FAIL: Broke fence regions legalization" << std::endl;
        return false;
    }

    std::vector<ophidian::circuit::Cell> nonFenceCells;
    std::copy_if(design.netlist().begin(ophidian::circuit::Cell()), design.netlist().end(ophidian::circuit::Cell()), std::back_inserter(nonFenceCells),
        [&](ophidian::circuit::Cell c) {
            return !design.placement().cellHasFence(c);
        }
    );

    cellLegalizer.buildRtree({design.netlist().begin(ophidian::circuit::Cell()), design.netlist().end(ophidian::circuit::Cell())});

    ophidian::legalization::FenceRegionIsolation fenceRegionIsolation(design);
    fenceRegionIsolation.isolateAllFenceCells();

    auto circuitBoundingBox = boost::geometry::model::box<ophidian::geometry::Point>(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint());

    for (auto cell : nonFenceCells) {
        if (cellDisplacement[cell] <= maxDisplacementAllowed) continue;
        bool result = cellLegalizer.legalizeCell(cell, findNewCellLocation(cell, circuitBoundingBox).toPoint(), circuitBoundingBox);
        count++;
        if (!result) fail_count++;
        // if (count > 0 && count % 100 == 0) std::cout << count << " cells processed (" << fail_count << " failed, displacement: " << cellDisplacement[cell] <<  ")" << std::endl;
    }

    std::cout << count << " cells processed (" << fail_count << " failed)" << std::endl;

    fenceRegionIsolation.restoreAllFenceCells();
    if (!CHECKDESIGN(design)) {
        std::cout << "FAIL: Broke legalization outside fences" << std::endl;
        return false;
    }

    boost::accumulators::accumulator_set<long long, boost::accumulators::features<boost::accumulators::tag::mean>> accAfter;

    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        accAfter(displacement(initialLocations[*cellIt], design.placement().cellLocation(*cellIt)));
    }

    auto afterMeanDisplacement = boost::accumulators::mean(accAfter);
    // auto afterMaxDisplacement = boost::accumulators::max(accAfter);
    // auto afterTotalDisplacement = boost::accumulators::sum(accAfter);

    DEBUG(afterMeanDisplacement);
    // DEBUG(afterMaxDisplacement);
    // DEBUG(afterTotalDisplacement);
}

int main(int argc, char const *argv[])
{
    std::vector<std::string> circuitNames = {
        "des_perf_1",
        "des_perf_a_md1",
        "des_perf_a_md2",
        "des_perf_b_md1",
        "des_perf_b_md2",
        "edit_dist_1_md1",
        "edit_dist_a_md2",
        "edit_dist_a_md3",
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
        std::cout << std::endl;
    }
}
