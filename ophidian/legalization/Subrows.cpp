#include "Subrows.h"
#include <cmath>
namespace ophidian
{
namespace legalization
{
Subrows::Subrows(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping)
    : netlist_(netlist), floorplan_(floorplan), placement_(placement), placementMapping_(placementMapping),
    subrowOrigins_(subrows_), subrowUpperCorners_(subrows_), subrowCapacities_(subrows_) {

}

void Subrows::createSubrows(util::MultiBox area, unsigned rowsPerCell, placement::RowAlignment alignment)
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

            geometry::Box subrowBox(rowOrigin.toPoint(), rowUpperRightCorner.toPoint());
            subrowsRtree_.insert(RtreeNode(subrowBox, subrow));

            rowOrigin.y(rowOrigin.y() + util::micrometer_t(rowHeight));
            rowUpperRightCorner.y(rowUpperRightCorner.y() + util::micrometer_t(rowHeight));
        }

    }

//    for (auto subrow : subrows_) {
//        auto rowOrigin = subrowOrigins_[subrow];
//        auto rowUpperRightCorner = subrowUpperCorners_[subrow];
//        geometry::Box subrowBox(rowOrigin.toPoint(), rowUpperRightCorner.toPoint());

//        std::vector<Subrow> intersectingSubrows;
//        for (auto otherSubrow : subrows_) {
//            auto otherSubrowOrigin = subrowOrigins_[otherSubrow];

//            if (rowOrigin.y() == otherSubrowOrigin.y() && rowUpperRightCorner.x() == otherSubrowOrigin.x() && subrow != otherSubrow) {
//                intersectingSubrows.push_back(otherSubrow);
//            }
//        }

//        if (!intersectingSubrows.empty()) {

//            util::Location newSubrowOrigin = rowOrigin;
//            util::Location newSubrowUpperCorner = rowUpperRightCorner;
//            for (auto intersectingSubrow : intersectingSubrows) {
//                auto intersectingSubrowOrigin = subrowOrigins_[intersectingSubrow];
//                auto intersectingSubrowUpperCorner = subrowUpperCorners_[intersectingSubrow];

//                newSubrowOrigin.x(util::micrometer_t(std::min(newSubrowOrigin.x(), intersectingSubrowOrigin.x())));
//                newSubrowOrigin.y(util::micrometer_t(std::min(newSubrowOrigin.y(), intersectingSubrowOrigin.y())));
//                newSubrowUpperCorner.x(util::micrometer_t(std::max(newSubrowUpperCorner.x(), intersectingSubrowUpperCorner.x())));
//                newSubrowUpperCorner.y(util::micrometer_t(std::max(newSubrowUpperCorner.y(), intersectingSubrowUpperCorner.y())));

//                geometry::Box intersectingSubrowBox(intersectingSubrowOrigin.toPoint(), intersectingSubrowUpperCorner.toPoint());
//                subrowsRtree_.remove(RtreeNode(intersectingSubrowBox, intersectingSubrow));
//                subrows_.erase(intersectingSubrow);
//            }

//            auto newSubrow = subrows_.add();
//            subrowOrigins_[newSubrow] = newSubrowOrigin;
//            subrowUpperCorners_[newSubrow] = newSubrowUpperCorner;
//            subrowCapacities_[newSubrow] = newSubrowUpperCorner.x() - newSubrowOrigin.x();

//            geometry::Box newSubrowBox(newSubrowOrigin.toPoint(), newSubrowUpperCorner.toPoint());
//            subrowsRtree_.insert(RtreeNode(newSubrowBox, newSubrow));

//            subrowsRtree_.remove(RtreeNode(subrowBox, subrow));
//            subrows_.erase(subrow);
//        }
//    }

//    for (auto subrow : subrows_) {
//        auto subrowOrigin = subrowOrigins_[subrow];
//        auto subrowUpperCorner = subrowUpperCorners_[subrow];

//        geometry::Box subrowBox(subrowOrigin.toPoint(), subrowUpperCorner.toPoint());
//        subrowsRtree_.remove(RtreeNode(subrowBox, subrow));

//        subrowOrigin.x(util::micrometer_t(std::ceil(subrowOrigin.toPoint().x() / siteWidth) * siteWidth));
//        subrowOrigin.y(util::micrometer_t(std::ceil(subrowOrigin.toPoint().y() / rowHeight) * rowHeight));
//        subrowUpperCorner.x(util::micrometer_t(std::ceil(subrowUpperCorner.toPoint().x() / siteWidth) * siteWidth));
//        subrowUpperCorner.y(util::micrometer_t(std::ceil(subrowUpperCorner.toPoint().y() / rowHeight) * rowHeight));

//        subrowOrigins_[subrow] = subrowOrigin;
//        subrowUpperCorners_[subrow] = subrowUpperCorner;
//        subrowCapacities_[subrow] = subrowUpperCorner.x() - subrowOrigin.x();

//        geometry::Box newSubrowBox(subrowOrigin.toPoint(), subrowUpperCorner.toPoint());
//        subrowsRtree_.insert(RtreeNode(newSubrowBox, subrow));
//    }

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt)
    {
        if (placement_.isFixed(*cellIt))
        {
            auto cellGeometry = placementMapping_.geometry(*cellIt);
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
                    double xUpperCorner = std::floor(units::unit_cast<double>(cellBox.min_corner().x() / siteWidth))*units::unit_cast<double>(siteWidth);
                    subrowUpperCorners_[leftSubrow] = util::Location(xUpperCorner, subrowNode.first.max_corner().y());
//                    subrowUpperCorners_[leftSubrow] = util::Location(cellBox.min_corner().x(), subrowNode.first.max_corner().y());
                    subrowCapacities_[leftSubrow] = subrowUpperCorners_[leftSubrow].x() - subrowOrigins_[leftSubrow].x();
                    geometry::Box leftSubrowBox(geometry::Point(units::unit_cast<double>(subrowOrigins_[leftSubrow].x()), units::unit_cast<double>(subrowOrigins_[leftSubrow].y())),
                                                geometry::Point(units::unit_cast<double>(subrowUpperCorners_[leftSubrow].x()), units::unit_cast<double>(subrowUpperCorners_[leftSubrow].y())));


                    auto rightSubrow = subrows_.add();
//                    auto rightSubrowX = std::max(cellBox.max_corner().x(), subrowNode.first.max_corner().x());
                    //alinhar origem
                    double xOrigin = std::ceil(units::unit_cast<double>(cellBox.max_corner().x() / siteWidth))*units::unit_cast<double>(siteWidth);
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
                    }
                    if (rightSubrowBox.max_corner().x() - rightSubrowBox.min_corner().x() > 0)
                    {
                        subrowsRtree_.insert(RtreeNode(rightSubrowBox, rightSubrow));
                    }

                }
            }
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

