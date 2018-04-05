#include "CellLegalizer.h"

#include <limits>
#include <unordered_set>
#include <boost/geometry/index/rtree.hpp>

namespace ophidian {
namespace legalization {

CellLegalizer::CellLegalizer(design::Design & design) :
    mDesign(design)
{
}

CellLegalizer::~CellLegalizer()
{
}

bool isNodeTouchingBox(CellLegalizer::RNode const& node, CellLegalizer::Box const& b) {
    auto node_box = std::get<0>(node);
    if (node_box.max_corner().x() <= b.min_corner().x() ||
        b.max_corner().x() <= node_box.min_corner().x() ||
        node_box.max_corner().y() <= b.min_corner().y() ||
        b.max_corner().y() <= node_box.min_corner().y()) {
        return false;
    }
    return true;
}

void CellLegalizer::buildRtree(const std::vector<circuit::Cell> legalizedCells) {
    mTree.clear();
    for (auto & cell : legalizedCells) {
        auto std_cell = mDesign.libraryMapping().cellStdCell(cell);

        auto cell_geometry = mDesign.library().geometry(std_cell)[0];

        auto cell_location_point = mDesign.placement().cellLocation(cell).toPoint();

        Box b(cell_location_point, Point(cell_location_point.x() + cell_geometry.max_corner().x(), cell_location_point.y() + cell_geometry.max_corner().y()));
        mTree.insert(std::make_pair(b, cell));
    }
}

long long CellLegalizer::legalizeCell(const circuit::Cell & targetCell, const geometry::Point & targetPosition, const std::vector<circuit::Cell> legalizedCells, const Box & legalizationRegion, bool estimateOnly)
{
    buildRtree(legalizedCells);
    return legalizeCell(targetCell, targetPosition, legalizationRegion, estimateOnly);
}

// Reuses the rtree
long long CellLegalizer::legalizeCell(const circuit::Cell & targetCell, const geometry::Point & targetPosition, const Box & legalizationRegion, bool estimateOnly)
{
    RTree tmpTree(mTree);

    auto std_cell = mDesign.libraryMapping().cellStdCell(targetCell);
    auto cell_geometry = mDesign.library().geometry(std_cell)[0];
    auto cell_location_point = mDesign.placement().cellLocation(targetCell).toPoint();

    Box targetBox(
        targetPosition,
        Point(targetPosition.x() + cell_geometry.max_corner().x(), targetPosition.y() + cell_geometry.max_corner().y())
    );

    Box originalBox(
        Point(cell_location_point.x(), cell_location_point.y()),
        Point(cell_location_point.x() + cell_geometry.max_corner().x(), cell_location_point.y() + cell_geometry.max_corner().y())
    );

    std::vector<RNode> possibleCells;
    tmpTree.query(boost::geometry::index::covered_by(originalBox), std::back_inserter(possibleCells));

    auto cell_node_it = std::find_if(possibleCells.begin(), possibleCells.end(), [&](const RNode &n) {
        return std::get<1>(n) == targetCell;
    });

    if (cell_node_it == possibleCells.end()) {
        return std::numeric_limits<long long>::max();
    }

    tmpTree.remove(*cell_node_it);

    // Fill a vector with all nodes that overlap the target box
    std::vector<RNode> overlaps_target;
    tmpTree.query(boost::geometry::index::intersects(targetBox) && boost::geometry::index::satisfies([&](RNode const& n) { return isNodeTouchingBox(n, targetBox); }), std::back_inserter(overlaps_target));

    RNode new_cell_node = std::make_pair(targetBox, targetCell);

    std::map<std::string, RNode> cellMovements;
    cellMovements[mDesign.netlist().name(targetCell)] = new_cell_node;

    tmpTree.insert(new_cell_node);

    // Vector with pairs of overlapping cells
    std::vector<std::pair<RNode, RNode>> overlaps;

    for (auto & n : overlaps_target) {
        if (mDesign.placement().isFixed(std::get<1>(n))) {
            return std::numeric_limits<long long>::max();
        }
        overlaps.push_back(make_pair(new_cell_node, n));
    }

    while (overlaps.size() > 0) {
        auto overlap = overlaps.back();
        overlaps.pop_back();

        auto node_i = std::get<0>(overlap);
        auto node_j = std::get<1>(overlap);

        RNode new_j;

        auto min_corner_i = std::get<0>(node_i).min_corner();
        auto max_corner_i = std::get<0>(node_i).max_corner();
        auto width_i = max_corner_i.x() - min_corner_i.x();

        auto min_corner_j = std::get<0>(node_j).min_corner();
        auto max_corner_j = std::get<0>(node_j).max_corner();
        auto width_j = max_corner_j.x() - min_corner_j.x();

        tmpTree.remove(node_j);
        Box target_j;

        bool moveLeft = (min_corner_j.x() + width_j/2) <= (targetBox.min_corner().x() + cell_geometry.max_corner().x()/2);

        if (moveLeft) {
            if ((min_corner_i.x() - width_j) < legalizationRegion.min_corner().x()) {
                return std::numeric_limits<long long>::max();
            } else {
                target_j = Box(
                    Point(min_corner_i.x() - width_j, min_corner_j.y()),
                    Point(min_corner_i.x(), max_corner_j.y())
                );
            }
        } else {
            if ((max_corner_i.x() + width_j) > legalizationRegion.max_corner().x()) {
                return std::numeric_limits<long long>::max();
            } else {
                target_j = Box(
                    Point(max_corner_i.x(), min_corner_j.y()),
                    Point(max_corner_i.x() + width_j, max_corner_j.y())
                );
            }
        }

        new_j = std::make_pair(target_j, std::get<1>(node_j));
        cellMovements[mDesign.netlist().name(std::get<1>(node_j))] = new_j;

        overlaps.erase(std::remove_if(overlaps.begin(), overlaps.end(), [&](auto & pair) {
            return std::get<1>(std::get<1>(pair)) == std::get<1>(node_j);
        }), overlaps.end());

        std::vector<RNode> overlaps_new_j;
        tmpTree.query(boost::geometry::index::intersects(target_j) && boost::geometry::index::satisfies([&](RNode const& n) { return isNodeTouchingBox(n, target_j); }), std::back_inserter(overlaps_new_j));
        for (auto & n : overlaps_new_j) {
            if (std::get<1>(new_j) == std::get<1>(n)) continue;
            if (mDesign.placement().isFixed(std::get<1>(n))) {
                return std::numeric_limits<long long>::max();
            }
            overlaps.push_back(make_pair(new_j, n));
        }

        tmpTree.insert(new_j);

    }

    long long movementDisplacement = 0;

    for (auto & n : cellMovements) {
        auto node = std::get<1>(n);

        double cellX = std::get<0>(node).min_corner().x();
        double cellY = std::get<0>(node).min_corner().y();

        auto initialCellLocationPoint = mDesign.placement().cellLocation(std::get<1>(node)).toPoint();

        movementDisplacement += std::abs(cellX - initialCellLocationPoint.x()) +
                                std::abs(cellY - initialCellLocationPoint.y());

        if (!estimateOnly) {
            ophidian::util::Location cellLocation(cellX, cellY);
            mDesign.placement().placeCell(std::get<1>(node), cellLocation);
        }
    }

    if (!estimateOnly) mTree = tmpTree;

    return movementDisplacement;
}

} // namespace legalization
} // namespace ophidian
