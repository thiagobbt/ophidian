#include "iccad2017SolutionQuality.h"

#include <algorithm>

namespace ophidian
{
namespace legalization
{

ICCAD2017SolutionQuality::ICCAD2017SolutionQuality(ophidian::design::Design &design, ophidian::design::Design &originalDesign):
    mInitialLocations(design.netlist().makeProperty<util::Location>(ophidian::circuit::Cell())),
    mInitialFixed(design.netlist().makeProperty<bool>(ophidian::circuit::Cell())),
    mDesign(design),
    mOriginalDesign(originalDesign),
    mRowHeight(mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y())
{
    for (auto cell : mDesign.netlist().range(circuit::Cell()))
    {
        mInitialFixed[cell] = originalDesign.placement().isFixed(cell);
        mInitialLocations[cell] = originalDesign.placement().cellLocation(cell);
    }
}

double ICCAD2017SolutionQuality::avgMovementScore() {
    struct statistics {
        double displacement = 0;
        unsigned quantity = 0;
    };

    std::unordered_map<double, statistics> cellHeightAccumulator;

    for (auto cell : mDesign.netlist().range(circuit::Cell())) {
        if (mDesign.placement().isFixed(cell)) continue;

        auto cellHeight = mDesign.placementMapping().geometry(cell)[0].max_corner().y() - mDesign.placementMapping().geometry(cell)[0].min_corner().y();
        double displacement = cellDisplacement(mDesign.placement().cellLocation(cell), mInitialLocations[cell]);

        cellHeightAccumulator[cellHeight].displacement += displacement;
        cellHeightAccumulator[cellHeight].quantity += 1;
    }

    double averageDisplacement = 0;

    for (const auto & element : cellHeightAccumulator) {
        const auto & stats = element.second;
        averageDisplacement += stats.displacement / stats.quantity;
    }

    averageDisplacement /= mRowHeight * cellHeightAccumulator.size();

    return averageDisplacement;
}

double ICCAD2017SolutionQuality::maxMovementScore() {
    return 1 + (maximumCellMovement()/100.0) * fmm();
}

double ICCAD2017SolutionQuality::hpwlScore() {
    double originalHPWL = 0;

    for (auto net : mOriginalDesign.netlist().range(circuit::Net())) {
        originalHPWL += hpwl(mOriginalDesign, net);
    }

    double currentHPWL = 0;

    for (auto net : mDesign.netlist().range(circuit::Net())) {
        currentHPWL += hpwl(mDesign, net);
    }

    double score = std::max((currentHPWL - originalHPWL) / originalHPWL, 0.0) * (1 + std::max(BETA * fof(), 0.2));

    return score;
}

double ICCAD2017SolutionQuality::totalDisplacement() {
    double displacement = 0.0;

    for (auto cell : mDesign.netlist().range(circuit::Cell())) {
        displacement += cellDisplacement(mDesign.placement().cellLocation(cell), mInitialLocations[cell]);
    }

    return displacement;
}

double ICCAD2017SolutionQuality::avgDisplacement() {
    double displacement = 0.0;
    unsigned int amount = 0;

    for (auto cell : mDesign.netlist().range(circuit::Cell())) {
        if (mInitialFixed[cell]) continue;

        displacement += cellDisplacement(mDesign.placement().cellLocation(cell), mInitialLocations[cell]);
        amount++;
    }

    return (displacement / mRowHeight) / amount;
}

int ICCAD2017SolutionQuality::maximumCellMovement() {
    double maximumCellMovement = 0;

    for (auto cell : mDesign.netlist().range(circuit::Cell())) {
        maximumCellMovement = std::max(maximumCellMovement, cellDisplacement(mDesign.placement().cellLocation(cell), mInitialLocations[cell]));
    }

    return maximumCellMovement / mRowHeight;
}

double ICCAD2017SolutionQuality::fmm() {
    std::string circuitName = mDesign.circuitName();
    double displacement = 0;

    for (auto cell : mDesign.netlist().range(circuit::Cell())) {
        if (cellDisplacement(mDesign.placement().cellLocation(cell), mInitialLocations[cell])/mRowHeight > mConstraints.at(circuitName).maximumMovement)
            displacement += cellDisplacement(mDesign.placement().cellLocation(cell), mInitialLocations[cell]);
    }

    displacement /= mRowHeight * mConstraints.at(circuitName).maximumMovement;
    return std::max(displacement, 1.0);
}

double ICCAD2017SolutionQuality::fof() {
    // TODO

    using Point = ophidian::geometry::Point;
    using Box = ophidian::geometry::Box;
    using RNode = std::pair<Box, ophidian::circuit::Cell>;
    using RTree = boost::geometry::index::rtree<RNode, boost::geometry::index::rstar<16>>;

    RTree tree;

    for (auto & cell : mDesign.netlist().range(circuit::Cell())) {
        auto std_cell = mDesign.libraryMapping().cellStdCell(cell);

        auto cell_geometry = mDesign.library().geometry(std_cell)[0];

        auto cell_location_point = mDesign.placement().cellLocation(cell).toPoint();

        Box b(cell_location_point, Point(cell_location_point.x() + cell_geometry.max_corner().x(), cell_location_point.y() + cell_geometry.max_corner().y()));
        tree.insert(std::make_pair(b, cell));
    }

    double totalOverflow = 0.0;

    auto singleBinArea = 64 * mRowHeight;
    double dMax = mConstraints.at(mDesign.circuitName()).maximumUtilization;
    double totalAreaMovableCells = 0.0;

    for (double y = 0; y < mDesign.floorplan().chipUpperRightCorner().toPoint().y() - (8 * mRowHeight); y += (8 * mRowHeight)) {
        for (double x = 0; x < mDesign.floorplan().chipUpperRightCorner().toPoint().x() - (8 * mRowHeight); x += (8 * mRowHeight)) {
            Box binBox({x, y}, {x + (8 * mRowHeight), y + (8 * mRowHeight)});
            std::vector<RNode> binCells;
            tree.query(boost::geometry::index::intersects(binBox), std::back_inserter(binCells));

            double movableArea = 0;
            double freeSpace = singleBinArea;

            for (auto & cellNode : binCells) {
                auto cell = cellNode.second;
                auto std_cell = mDesign.libraryMapping().cellStdCell(cell);
                auto cell_geometry = mDesign.library().geometry(std_cell);

                double covered_area = 0;

                for(auto cell_box : cell_geometry) {
                    geometry::Box intersection;
                    boost::geometry::intersection(cell_box, binBox, intersection);
                    covered_area += boost::geometry::area(intersection);
                }

                if (mDesign.placement().isFixed(cell)) {
                    freeSpace -= covered_area;
                } else {
                    movableArea += covered_area;
                }
            }

            double overflow = movableArea - freeSpace * dMax;
            totalOverflow += overflow;
        }
    }

    return totalOverflow * singleBinArea * dMax * totalAreaMovableCells;
}

double ICCAD2017SolutionQuality::hpwl(ophidian::design::Design & design, const ophidian::circuit::Net & net) {
    double minX = std::numeric_limits<double>::max(),
           minY = std::numeric_limits<double>::max(),
           maxX = std::numeric_limits<double>::lowest(),
           maxY = std::numeric_limits<double>::lowest();

    for (auto pin : design.netlist().pins(net)) {
        util::Location pinLocation;

        auto input = design.netlist().input(pin);
        auto output = design.netlist().output(pin);

        if (input != circuit::Input()) {
            pinLocation = design.placement().inputPadLocation(input);
        } else if (output != circuit::Output()) {
            pinLocation = design.placement().outputPadLocation(output);
        } else {
            pinLocation = design.placementMapping().location(pin);
        }

        minX = std::min(pinLocation.toPoint().x(), minX);
        minY = std::min(pinLocation.toPoint().y(), minY);
        maxX = std::max(pinLocation.toPoint().x(), maxX);
        maxY = std::max(pinLocation.toPoint().y(), maxY);
    }
    return std::abs(maxX - minX) + std::abs(maxY - minY);
}


double ICCAD2017SolutionQuality::hpwl(const circuit::Net &net) {
    return hpwl(mDesign, net);
}

} // namespace legalization
} // namespace ophidian
