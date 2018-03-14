#include "Subrows.h"
#include <cmath>
namespace ophidian
{
namespace legalization
{
Subrows::Subrows(design::Design & design)
    : netlist_(design.netlist()), floorplan_(design.floorplan()), placement_(design.placement()), placementMapping_(design.placementMapping()),
    subrowOrigins_(subrows_), subrowUpperCorners_(subrows_), subrowCapacities_(subrows_) {

}

Subrows::Subrows(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping)
    : netlist_(netlist), floorplan_(floorplan), placement_(placement), placementMapping_(placementMapping),
    subrowOrigins_(subrows_), subrowUpperCorners_(subrows_), subrowCapacities_(subrows_) {

}

void Subrows::createSubrows(const std::vector<circuit::Cell> &cells, util::MultiBox area, unsigned rowsPerCell, placement::RowAlignment alignment, bool align)
{
    subrows_.clear();
    subrowsRtree_.clear();

    auto singleRowHeight = units::unit_cast<double>(floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).y());
    auto siteWidth = units::unit_cast<double>(floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).x());
    auto rowHeight = units::unit_cast<double>(floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).y()) * rowsPerCell;

    for (auto box : area) {
        double rowOriginX = std::ceil(box.min_corner().x() / siteWidth) * siteWidth;
        double rowOriginY = std::ceil(box.min_corner().y() / singleRowHeight) * singleRowHeight;

        placement::RowAlignment boxAlignment = (((int)(rowOriginY / singleRowHeight) % 2) == 0) ? placement::RowAlignment::EVEN : placement::RowAlignment::ODD;
        auto rowIncrement = (alignment != placement::RowAlignment::NA && alignment != boxAlignment) ? singleRowHeight : 0;

        auto rowOrigin = util::Location(rowOriginX, rowOriginY + rowIncrement);

        double rowUpperX = std::floor(box.max_corner().x() / siteWidth) * siteWidth;
        auto rowUpperRightCorner = util::Location(rowUpperX, units::unit_cast<double>(rowOrigin.y()) + rowHeight);

        while (units::unit_cast<double>(rowUpperRightCorner.y()) <= box.max_corner().y()) {
            auto subrow = subrows_.add();

            subrowOrigins_[subrow] = rowOrigin;
            subrowUpperCorners_[subrow] = rowUpperRightCorner;
            subrowCapacities_[subrow] = rowUpperRightCorner.x() - rowOrigin.x();

            if (subrowOrigins_[subrow].toPoint().x() == 208200 && subrowOrigins_[subrow].toPoint().y() == 354000
                    && subrowUpperCorners_[subrow].toPoint().x() == 256200 && subrowUpperCorners_[subrow].toPoint().y() == 356000
                    && util::Debug::mDebug) {
                std::cout << "stop " << std::endl;
            }

            geometry::Box subrowBox(rowOrigin.toPoint(), rowUpperRightCorner.toPoint());
            subrowsRtree_.insert(RtreeNode(subrowBox, subrow));

            rowOrigin.y(rowOrigin.y() + util::micrometer_t(rowHeight));
            rowUpperRightCorner.y(rowUpperRightCorner.y() + util::micrometer_t(rowHeight));
        }

    }

    for (auto cell : cells)
    {
        if (placement_.isFixed(cell))
        {
            auto cellName = netlist_.name(cell);
            auto cellGeometry = placementMapping_.geometry(cell);
            for (auto cellBox : cellGeometry)
            {
                std::vector<RtreeNode> intersectingSubrowNodes;
                subrowsRtree_.query(boost::geometry::index::overlaps(cellBox), std::back_inserter(intersectingSubrowNodes));
                subrowsRtree_.query(boost::geometry::index::covers(cellBox), std::back_inserter(intersectingSubrowNodes));
                subrowsRtree_.query(boost::geometry::index::covered_by(cellBox), std::back_inserter(intersectingSubrowNodes));

//                std::set<RtreeNode> intersectingSubrowNodesSet(intersectingSubrowNodes.begin(), intersectingSubrowNodes.end(), RtreeNodeComparator());
                std::set<RtreeNode, RtreeNodeComparator> intersectingSubrowNodesSet;
                for (auto subrowNode : intersectingSubrowNodes) {
                    intersectingSubrowNodesSet.insert(subrowNode);
                }
                for (auto subrowNode : intersectingSubrowNodesSet)
                {
                    auto subrow = subrowNode.second;

                    auto leftSubrow = subrows_.add();
                    subrowOrigins_[leftSubrow] = subrowOrigins_[subrow];
//                    auto leftSubrowX = std::max(cellBox.min_corner().x(), subrowNode.first.min_corner().x());
                    //alinhar upperCorner
                    auto siteWidth = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).x();
                    double xUpperCorner = cellBox.min_corner().x();
                    if (align) {
                        xUpperCorner = std::floor(units::unit_cast<double>(cellBox.min_corner().x() / siteWidth))*units::unit_cast<double>(siteWidth);
                    }
                    subrowUpperCorners_[leftSubrow] = util::Location(xUpperCorner, subrowNode.first.max_corner().y());
//                    subrowUpperCorners_[leftSubrow] = util::Location(cellBox.min_corner().x(), subrowNode.first.max_corner().y());
                    subrowCapacities_[leftSubrow] = subrowUpperCorners_[leftSubrow].x() - subrowOrigins_[leftSubrow].x();
                    geometry::Box leftSubrowBox(geometry::Point(units::unit_cast<double>(subrowOrigins_[leftSubrow].x()), units::unit_cast<double>(subrowOrigins_[leftSubrow].y())),
                                                geometry::Point(units::unit_cast<double>(subrowUpperCorners_[leftSubrow].x()), units::unit_cast<double>(subrowUpperCorners_[leftSubrow].y())));


                    auto rightSubrow = subrows_.add();
//                    auto rightSubrowX = std::max(cellBox.max_corner().x(), subrowNode.first.max_corner().x());
                    //alinhar origem
                    double xOrigin = cellBox.max_corner().x();
                    if (align) {
                        xOrigin = std::ceil(units::unit_cast<double>(cellBox.max_corner().x() / siteWidth))*units::unit_cast<double>(siteWidth);
                    }
//                    subrowOrigins_[rightSubrow] = util::Location(cellBox.max_corner().x(), subrowNode.first.min_corner().y());
                    subrowOrigins_[rightSubrow] = util::Location(xOrigin, subrowNode.first.min_corner().y());
                    subrowUpperCorners_[rightSubrow] = subrowUpperCorners_[subrow];
                    subrowCapacities_[rightSubrow] = subrowUpperCorners_[rightSubrow].x() - subrowOrigins_[rightSubrow].x();
                    geometry::Box rightSubrowBox(geometry::Point(units::unit_cast<double>(subrowOrigins_[rightSubrow].x()), units::unit_cast<double>(subrowOrigins_[rightSubrow].y())),
                                                 geometry::Point(units::unit_cast<double>(subrowUpperCorners_[rightSubrow].x()), units::unit_cast<double>(subrowUpperCorners_[rightSubrow].y())));


                    subrowsRtree_.remove(subrowNode);
                    subrows_.erase(subrow);
                    if (leftSubrowBox.max_corner().x() - leftSubrowBox.min_corner().x() > 0)
                    {
                        subrowsRtree_.insert(RtreeNode(leftSubrowBox, leftSubrow));
                    } else {
                        subrows_.erase(leftSubrow);
                    }
                    if (rightSubrowBox.max_corner().x() - rightSubrowBox.min_corner().x() > 0)
                    {
                        subrowsRtree_.insert(RtreeNode(rightSubrowBox, rightSubrow));
                    } else {
                        subrows_.erase(rightSubrow);
                    }

                }
            }
        }
    }

    if (util::Debug::mDebug) {
        for (auto subrow : subrows_) {
            std::cout << "subrow " << subrowOrigins_[subrow].x() << ", " << subrowOrigins_[subrow].y() << " -> "
                                          << subrowUpperCorners_[subrow].x() << ", " << subrowUpperCorners_[subrow].y() << std::endl;
        }
    }
}

ophidian::util::Range<Subrows::SubrowsIterator> Subrows::range(Subrow) const
{
    return ophidian::util::Range<SubrowsIterator>(subrows_.begin(), subrows_.end());
}

void Subrows::findClosestSubrows(unsigned numberOfSubrows, util::Location point, std::vector<Subrow> &subrows) const
{
    geometry::Point doublePoint(units::unit_cast<double>(point.x()), units::unit_cast<double>(point.y()));
    std::vector<RtreeNode> closeSubrowNodes;
    subrowsRtree_.query(boost::geometry::index::nearest(doublePoint, numberOfSubrows), std::back_inserter(closeSubrowNodes));

    subrows.reserve(closeSubrowNodes.size());
    for (auto rtreeNode : closeSubrowNodes)
    {
        subrows.push_back(rtreeNode.second);
    }
}

Subrow Subrows::findContainedSubrow(util::Location location) const
{
    std::vector<RtreeNode> closeSubrowNodes;
    subrowsRtree_.query(boost::geometry::index::contains(location.toPoint()), std::back_inserter(closeSubrowNodes));

    if (closeSubrowNodes.empty()) {
        std::vector<Subrow> closeSubrows;
        findClosestSubrows(2, location, closeSubrows);
        for (auto subrow : closeSubrows) {
            auto subrowOrigin = subrowOrigins_[subrow];
            if (subrowOrigin.y() == location.y()) {
                return subrow;
            }
        }
    } else {
        return closeSubrowNodes.front().second;
    }

}

Subrow Subrows::findContainedSubrow(geometry::Box cellBox) const
{
    std::vector<RtreeNode> closeSubrowNodes;
    subrowsRtree_.query(boost::geometry::index::contains(cellBox), std::back_inserter(closeSubrowNodes));
    subrowsRtree_.query(boost::geometry::index::overlaps(cellBox), std::back_inserter(closeSubrowNodes));

    if (closeSubrowNodes.empty()) {
        std::vector<Subrow> closeSubrows;
        findClosestSubrows(1, util::Location(cellBox.min_corner().x(), cellBox.min_corner().y()), closeSubrows);
        return closeSubrows.front();
    } else {
        return closeSubrowNodes.front().second;
    }

}

void Subrows::findContainedSubrows(geometry::Box cellBox, std::vector<Subrow> &subrows) const
{
    std::vector<RtreeNode> closeSubrowNodes;
    subrowsRtree_.query(boost::geometry::index::contains(cellBox), std::back_inserter(closeSubrowNodes));
    subrowsRtree_.query(boost::geometry::index::overlaps(cellBox), std::back_inserter(closeSubrowNodes));

    if (closeSubrowNodes.empty()) {
        findClosestSubrows(1, util::Location(cellBox.min_corner().x(), cellBox.min_corner().y()), subrows);
    } else {
        for (auto subrowNode : closeSubrowNodes) {
            subrows.push_back(subrowNode.second);
        }
    }
}

bool Subrows::isInsideSubrows(geometry::Box cellBox)
{
    std::vector<RtreeNode> closeSubrowNodes;
    subrowsRtree_.query(boost::geometry::index::contains(cellBox), std::back_inserter(closeSubrowNodes));
    subrowsRtree_.query(boost::geometry::index::overlaps(cellBox), std::back_inserter(closeSubrowNodes));

    auto cellArea = boost::geometry::area(cellBox);
    double intersectionArea = 0;
    for (auto node : closeSubrowNodes) {
        geometry::Box intersectionBox;
        boost::geometry::intersection(node.first, cellBox, intersectionBox);
        intersectionArea += boost::geometry::area(intersectionBox);
    }
    return cellArea == intersectionArea;
}

util::micrometer_t Subrows::capacity(Subrow subrow) const
{
    return subrowCapacities_[subrow];
}

void Subrows::capacity(Subrow subrow, util::micrometer_t capacity)
{
    subrowCapacities_[subrow] = capacity;
}

util::Location Subrows::origin(Subrow subrow) const
{
    return subrowOrigins_[subrow];
}

util::Location Subrows::upperCorner(Subrow subrow) const
{
    return subrowUpperCorners_[subrow];
}
} // namespace legalization
} // namespace ophidian

