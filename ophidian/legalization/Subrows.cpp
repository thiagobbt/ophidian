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

void Subrows::createSubrows(util::MultiBox area, unsigned rowsPerCell, unsigned rowIndex)
{
    subrows_.clear();
    subrowsRtree_.clear();

    auto singleRowHeight = units::unit_cast<double>(floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).y());
    auto siteWidth = units::unit_cast<double>(floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).x());
    auto rowHeight = units::unit_cast<double>(floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).y()) * rowsPerCell;

    for (auto box : area) {

//        std::cout << "division " << (box.min_corner().y() / singleRowHeight) << std::endl;
//        std::cout << "box origin " << box.min_corner().x() << ", " << box.min_corner().y() << std::endl;
//        std::cout << "box in even origin " << boxInEvenOrigin << std::endl;

        double rowOriginX = std::ceil(box.min_corner().x() / siteWidth) * siteWidth;
        double rowOriginY = std::ceil(box.min_corner().y() / singleRowHeight) * singleRowHeight;

        bool boxInEvenOrigin = ((int)(rowOriginY / singleRowHeight) % 2) == 0;
        auto rowIncrement = rowIndex * singleRowHeight;
        if (!boxInEvenOrigin) {
            rowIncrement = (rowIndex) ? 0 : singleRowHeight;
        }

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
//    for (auto rowIt = floorplan_.rowsRange().begin(); rowIt != floorplan_.rowsRange().end(); ++rowIt)
//    {
//        if (rowIndex % rowsPerCell == 0)
//        {
//            auto subrow = subrows_.add();
//            auto rowOrigin = floorplan_.origin(*rowIt);

//            auto rowUpperCorner = floorplan_.rowUpperRightCorner(*rowIt);
//            rowUpperCorner.x(rowOrigin.x() + rowUpperCorner.x());
//            rowUpperCorner.y(rowOrigin.y() + (rowsPerCell * rowUpperCorner.y()));
//            subrowOrigins_[subrow] = rowOrigin;
//            subrowUpperCorners_[subrow] = rowUpperCorner;
//            subrowCapacities_[subrow] = rowUpperCorner.x() - rowOrigin.x();

//            geometry::Box subrowBox(geometry::Point(units::unit_cast<double>(rowOrigin.x()), units::unit_cast<double>(rowOrigin.y())),
//                                    geometry::Point(units::unit_cast<double>(rowUpperCorner.x()), units::unit_cast<double>(rowUpperCorner.y())));
//            subrowsRtree_.insert(RtreeNode(subrowBox, subrow));
//        }
//        rowIndex++;
//    }

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt)
    {
        if (placement_.isFixed(*cellIt))
        {
//            std::string cellName = netlist_.name(*cellIt);
//            if (cellName.find("FE_OCPC56160_FE_OFN47351_n_22922") != std::string::npos)
//            {
//                std::cout << "stop" << std::endl;
//            }

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


//                    if (subrowNode.first.min_corner().y() == 160000 && rowsPerCell == 1) {
//                        std::cout << "cell box " << cellBox.min_corner().x() << ", " << cellBox.min_corner().y() << " -> " <<
//                                     cellBox.max_corner().x() << ", " << cellBox.max_corner().y() << std::endl;
//                        std::cout << "original subrow " << subrowOrigins_[subrow].x() << ", " << subrowOrigins_[subrow].y() << " -> " <<
//                                  subrowUpperCorners_[subrow].x() << ", " << subrowUpperCorners_[subrow].y() << std::endl;
//                        std::cout << "left subrow " << subrowOrigins_[leftSubrow].x() << ", " << subrowOrigins_[leftSubrow].y() << " -> " <<
//                                  subrowUpperCorners_[leftSubrow].x() << ", " << subrowUpperCorners_[leftSubrow].y() << std::endl;
//                        std::cout << "right subrow " << subrowOrigins_[rightSubrow].x() << ", " << subrowOrigins_[rightSubrow].y() << " -> " <<
//                                  subrowUpperCorners_[rightSubrow].x() << ", " << subrowUpperCorners_[rightSubrow].y() << std::endl << std::endl;
//                    }

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
//    std::cout << "ROW PER CELL " <<rowsPerCell<<std::endl;
//    for(auto subRowIt = subrows_.begin(); subRowIt != subrows_.end(); ++subRowIt)
//    {
////        std::cout << "sub: x: " << subrowOrigins_[*subRowIt].x() <<" y: "<< subrowOrigins_[*subRowIt].y() << " -> "
////                  << subrowUpperCorners_[*subRowIt].x() << " " << subrowUpperCorners_[*subRowIt].y() <<std::endl;

//        util::Location origin = subrowOrigins_[*subRowIt];
//        util::Location upperCorner = subrowUpperCorners_[*subRowIt];
//        std::cout << "<rect "
//                  <<"x=\""<< units::unit_cast<double>(origin.x())/1000 << "\" y=\""<< units::unit_cast<double>(origin.y())/1000 << "\""
//                  <<" width=\""<< units::unit_cast<double>(upperCorner.x() - origin.x())/1000 <<"\" height=\""<< units::unit_cast<double>(upperCorner.y() - origin.y())/1000 <<"\""
//                  <<" style=\"fill:white;stroke:black;stroke-width:0.01;fill-opacity:0.1;stroke-opacity:0.9\" />" << std::endl;
//    }

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

