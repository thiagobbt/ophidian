#include <cstdlib>
#include <iostream>
#include <limits>
#include <chrono>

#include <boost/accumulators/accumulators.hpp>
#include <boost/bind.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>
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

#include <ophidian/legalization/iccad2017SolutionQuality.h>

#define DEBUG(x) [&]() { std::cerr << #x << ":" << x << std::endl; }()
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

bool optimizeCircuit(const std::string & circuitName, float maxDisplacementFactor = 1.0, int numPositions = 10) {
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

    auto start = std::chrono::high_resolution_clock::now();

    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::util::Location> initialLocations(design.netlist().makeProperty<ophidian::util::Location>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::util::Location> initialLegalizedLocations(design.netlist().makeProperty<ophidian::util::Location>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, std::string> initialOrientations(design.netlist().makeProperty<std::string>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, long long> cellDisplacement(design.netlist().makeProperty<long long>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::circuit::Cell> cellMapper(design.netlist().makeProperty<ophidian::circuit::Cell>(ophidian::circuit::Cell()));

    for (auto cellIt = originalDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != originalDesign.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        auto originalCell = *cellIt;
        auto currentCell = design.netlist().find(ophidian::circuit::Cell(), originalDesign.netlist().name(originalCell));

        cellMapper[originalCell] = currentCell;

        initialLocations[currentCell] = originalDesign.placement().cellLocation(originalCell);
        initialLegalizedLocations[currentCell] = design.placement().cellLocation(currentCell);
        initialOrientations[currentCell] = originalDesign.placement().cellOrientation(originalCell);
        cellDisplacement[currentCell] = displacement(initialLocations[currentCell], design.placement().cellLocation(currentCell));
    }

    boost::accumulators::accumulator_set<long long, boost::accumulators::features<boost::accumulators::tag::variance, boost::accumulators::tag::mean, boost::accumulators::tag::max, boost::accumulators::tag::sum>> accBefore;
    for (auto displacement : cellDisplacement) {
        accBefore(displacement);
    }

    auto stdDevDisplacement = sqrt(boost::accumulators::variance(accBefore));

    auto meanDisplacement  = boost::accumulators::mean(accBefore);
    auto maxDisplacement   = boost::accumulators::max(accBefore);
    auto totalDisplacement = boost::accumulators::sum(accBefore);

    auto maxDisplacementAllowed = meanDisplacement + (stdDevDisplacement * maxDisplacementFactor);

    std::cout << "original mean displacement:"  << meanDisplacement << std::endl;
    std::cout << "original max displacement:"   << maxDisplacement << std::endl;
    std::cout << "original total displacement:" << totalDisplacement << std::endl;
    std::cout << "max displacement allowed:"    << maxDisplacementAllowed << std::endl;

    ophidian::legalization::CellLegalizer cellLegalizer(design);

    ophidian::legalization::RectilinearFences rectilinearFences(design);
    rectilinearFences.addBlocksToRectilinearFences();

    auto findNewCellLocationsMulti = [&](ophidian::circuit::Cell & cell, ophidian::util::MultiBox & area) {
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

        std::vector<ophidian::util::Location> newCellLocations;

        auto cellBox = [&](ophidian::util::Location loc) {
            return ophidian::geometry::Box{loc.toPoint(), ophidian::geometry::Point{loc.toPoint().x() + cellWidth, loc.toPoint().y() + cellHeight}};
        };

        auto within = [&](ophidian::geometry::Box & cell_box, ophidian::util::MultiBox & fence_area) {
            double coveredArea = 0;
            for (auto fence_box : fence_area) {
                if (boost::geometry::intersects(cell_box, fence_box)) {
                    ophidian::geometry::Box intersection;
                    boost::geometry::intersection(cell_box, fence_box, intersection);
                    coveredArea += boost::geometry::area(intersection);
                }
            }

            auto cellArea = boost::geometry::area(cell_box);
            return coveredArea == cellArea;
        };

        auto checkPosition = [&](ophidian::util::Location loc) {
            auto box = cellBox(loc);
            return within(box, area);
        };

        enum direction {UP = 0, RIGHT = 1, DOWN = 2, LEFT = 3};
        direction currentDirection = static_cast<direction>(0);
        int count = 0;
        int maxCount = 1;

        auto currentX = newX;
        auto currentY = newY;

        if (checkPosition({currentX, currentY})) newCellLocations.emplace_back(currentX, currentY);

        while (newCellLocations.size() < numPositions) {
            switch (currentDirection) {
                case UP:
                    currentY = currentY + (rowHeight * 2);
                    break;
                case RIGHT:
                    currentX = currentX + siteWidth;
                    break;
                case DOWN:
                    currentY = currentY - (rowHeight * 2);
                    break;
                case LEFT:
                    currentX = currentX - siteWidth;
                    break;
            }

            if (checkPosition({currentX, currentY})) newCellLocations.emplace_back(currentX, currentY);

            count++;
            if (count == maxCount) {
                count = 0;
                int newDirection = (static_cast<int>(currentDirection) + 1) % 4;
                currentDirection = static_cast<direction>(newDirection);
                if (currentDirection == UP or currentDirection == DOWN) {
                    maxCount += 1;
                }
            }
        }

        return newCellLocations;
    };

    int fail_count = 0;
    int count = 0;
    int fence_num = 0;

    for (auto fence : design.fences().range()) {
        auto fenceArea = design.fences().area(fence);
        ophidian::geometry::Box fenceBoundingBox;
        boost::geometry::envelope(fenceArea.toMultiPolygon(), fenceBoundingBox);

        auto cellsInFence = design.fences().members(fence);
        cellLegalizer.buildRtree({cellsInFence.begin(), cellsInFence.end()});

        for (auto cell : design.fences().members(fence)) {
            if (cellDisplacement[cell] <= maxDisplacementAllowed) continue;

            auto possibleCellLocations = findNewCellLocationsMulti(cell, fenceArea);
            std::vector<long long> possibleCellLocationsDisplacement;

            for (auto possibleLocation : possibleCellLocations) {
                auto result = cellLegalizer.legalizeCell(cell, possibleLocation.toPoint(), fenceBoundingBox, true);
                possibleCellLocationsDisplacement.push_back(result);
            }

            auto minDisplacement = std::min_element(possibleCellLocationsDisplacement.begin(), possibleCellLocationsDisplacement.end());
            auto bestLocation = possibleCellLocations[minDisplacement - possibleCellLocationsDisplacement.begin()];

            auto result = cellLegalizer.legalizeCell(cell, bestLocation.toPoint(), fenceBoundingBox, false);
            count++;
            if (result == std::numeric_limits<long long>::max()) fail_count++;
        }
    }

    std::cout << "fence cells processed:" << count << std::endl;
    std::cout << "fence cells failed:" << fail_count << std::endl;

    rectilinearFences.eraseBlocks();

    std::vector<ophidian::circuit::Cell> nonFenceCells;
    std::copy_if(design.netlist().begin(ophidian::circuit::Cell()), design.netlist().end(ophidian::circuit::Cell()), std::back_inserter(nonFenceCells),
        [&](ophidian::circuit::Cell c) {
            return !design.placement().cellHasFence(c);
        }
    );

    cellLegalizer.buildRtree({design.netlist().begin(ophidian::circuit::Cell()), design.netlist().end(ophidian::circuit::Cell())});

    ophidian::legalization::FenceRegionIsolation fenceRegionIsolation(design);
    fenceRegionIsolation.isolateAllFenceCells();

    auto circuitBoundingBox = ophidian::geometry::Box(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint());
    auto circuitMultiBox = ophidian::util::MultiBox({circuitBoundingBox});

    for (auto cell : nonFenceCells) {
        if (cellDisplacement[cell] <= maxDisplacementAllowed) continue;

        auto possibleCellLocations = findNewCellLocationsMulti(cell, circuitMultiBox);
        std::vector<long long> possibleCellLocationsDisplacement;

        for (auto possibleLocation : possibleCellLocations) {
            auto result = cellLegalizer.legalizeCell(cell, possibleLocation.toPoint(), circuitBoundingBox, true);
            possibleCellLocationsDisplacement.push_back(result);
        }

        auto minDisplacement = std::min_element(possibleCellLocationsDisplacement.begin(), possibleCellLocationsDisplacement.end());
        auto bestLocation = possibleCellLocations[minDisplacement - possibleCellLocationsDisplacement.begin()];

        auto result = cellLegalizer.legalizeCell(cell, bestLocation.toPoint(), circuitBoundingBox, false);

        count++;
        if (!result) fail_count++;
    }

    auto finish = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = finish - start;

    std::cout << "total cells processed:" << count << std::endl;
    std::cout << "total cells failed:" << fail_count << std::endl;

    fenceRegionIsolation.restoreAllFenceCells();
    if (!CHECKDESIGN(design)) {
        std::cout << "FAIL: Broke legalization outside fences" << std::endl;
        return false;
    }

    boost::accumulators::accumulator_set<long long, boost::accumulators::features<boost::accumulators::tag::max, boost::accumulators::tag::mean, boost::accumulators::tag::sum>> accAfter;

    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); ++cellIt)
    {
        accAfter(displacement(initialLocations[*cellIt], design.placement().cellLocation(*cellIt)));
    }

    auto afterMeanDisplacement  = boost::accumulators::mean(accAfter);
    auto afterMaxDisplacement   = boost::accumulators::max(accAfter);
    auto afterTotalDisplacement = boost::accumulators::sum(accAfter);

    std::cout << "after mean displacement:"  << afterMeanDisplacement << std::endl;
    std::cout << "after max displacement:"   << afterMaxDisplacement << std::endl;
    std::cout << "after total displacement:" << afterTotalDisplacement << std::endl;
    std::cout << "runtime (seconds):" << elapsed.count() << std::endl;

    int numMovedCells = 0;

    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); ++cellIt) {
        auto cell = *cellIt;
        auto cellLocation = design.placement().cellLocation(cell);
        auto initialCellLocation = initialLegalizedLocations[cell];

        if (cellLocation != initialCellLocation) numMovedCells++;
    }

    std::cout << "number of moved cells:" << numMovedCells << std::endl;


    ophidian::legalization::ICCAD2017SolutionQuality quality(design, originalDesign);
    std::cout << "ICCAD2017 quality:" << quality.rawScore() << std::endl;
}

int main(int argc, char const *argv[])
{
    float maxDisplacementFactor = 1.0;
    int numPositions = 10;

    if (argc >= 2) {
        maxDisplacementFactor = std::atof(argv[1]);
    }

    if (argc >= 3) {
        numPositions = std::atoi(argv[2]);
    }

    std::cout << "Using " << maxDisplacementFactor << " std deviations above mean as cutoff displacement, with " << numPositions << " positions" << std::endl;

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

    auto totalRuntimeStart = std::chrono::high_resolution_clock::now();

    for (auto circuitName : circuitNames) {
        std::cout << circuitName << std::endl;

        optimizeCircuit(circuitName, maxDisplacementFactor, numPositions);

        std::cout << std::endl;
    }

    auto totalRuntimeFinish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTotalRuntime = totalRuntimeFinish - totalRuntimeStart;

    std::cout << "total runtime (seconds):" << elapsedTotalRuntime.count() << std::endl;
}
