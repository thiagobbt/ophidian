#include "Subrows.h"

namespace ophidian {
namespace legalization {
Subrows::Subrows(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping)
    : netlist_(netlist), floorplan_(floorplan), placement_(placement), placementMapping_(placementMapping),
    subrowOrigins_(subrows_), subrowUpperCorners_(subrows_), subrowCapacities_(subrows_) {
    createSubrows();
}

void Subrows::createSubrows()
{
    for (auto rowIt = floorplan_.rowsRange().begin(); rowIt != floorplan_.rowsRange().end(); ++rowIt) {
        auto subrow = subrows_.add();
        auto rowOrigin = floorplan_.origin(*rowIt);
        auto rowUpperCorner = floorplan_.rowUpperRightCorner(*rowIt);
        rowUpperCorner.x(rowUpperCorner.x() + rowOrigin.x());
        rowUpperCorner.y(rowUpperCorner.y() + rowOrigin.y());
        subrowOrigins_[subrow] = rowOrigin;
        subrowUpperCorners_[subrow] = rowUpperCorner;
        subrowCapacities_[subrow] = rowUpperCorner.x() - rowOrigin.x();

        geometry::Box subrowBox(geometry::Point(units::unit_cast<double>(rowOrigin.x()), units::unit_cast<double>(rowOrigin.y())),
                                geometry::Point(units::unit_cast<double>(rowUpperCorner.x()), units::unit_cast<double>(rowUpperCorner.y())));
        subrowsRtree_.insert(RtreeNode(subrowBox, subrow));
    }

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        if (placement_.isFixed(*cellIt)) {
            auto cellGeometry = placementMapping_.geometry(*cellIt);
            for (auto cellBox : cellGeometry) {
                std::vector<RtreeNode> intersectingSubrowNodes;
                subrowsRtree_.query(boost::geometry::index::intersects(cellBox), std::back_inserter(intersectingSubrowNodes));
                for (auto subrowNode : intersectingSubrowNodes) {
                    if (cellBox.min_corner().y() == subrowNode.first.min_corner().y()) {
                        auto subrow = subrowNode.second;

                        auto leftSubrow = subrows_.add();
                        subrowOrigins_[leftSubrow] = subrowOrigins_[subrow];
                        subrowUpperCorners_[leftSubrow] = util::Location(cellBox.min_corner().x(), cellBox.max_corner().y());
                        subrowCapacities_[leftSubrow] = subrowUpperCorners_[leftSubrow].x() - subrowOrigins_[leftSubrow].x();
                        geometry::Box leftSubrowBox(geometry::Point(units::unit_cast<double>(subrowOrigins_[leftSubrow].x()), units::unit_cast<double>(subrowOrigins_[leftSubrow].y())),
                                                geometry::Point(units::unit_cast<double>(subrowUpperCorners_[leftSubrow].x()), units::unit_cast<double>(subrowUpperCorners_[leftSubrow].y())));

                        auto rightSubrow = subrows_.add();
                        subrowOrigins_[rightSubrow] = util::Location(cellBox.max_corner().x(), cellBox.min_corner().y());
                        subrowUpperCorners_[rightSubrow] = subrowUpperCorners_[subrow];
                        subrowCapacities_[rightSubrow] = subrowUpperCorners_[rightSubrow].x() - subrowOrigins_[rightSubrow].x();
                        geometry::Box rightSubrowBox(geometry::Point(units::unit_cast<double>(subrowOrigins_[rightSubrow].x()), units::unit_cast<double>(subrowOrigins_[rightSubrow].y())),
                                                geometry::Point(units::unit_cast<double>(subrowUpperCorners_[rightSubrow].x()), units::unit_cast<double>(subrowUpperCorners_[rightSubrow].y())));

                        subrowsRtree_.remove(subrowNode);
                        subrowsRtree_.insert(RtreeNode(leftSubrowBox, leftSubrow));
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
    for (auto rtreeNode : closeSubrowNodes) {
        subrows.push_back(rtreeNode.second);
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
}
}

