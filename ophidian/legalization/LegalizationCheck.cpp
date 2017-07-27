#include "LegalizationCheck.h"

namespace ophidian
{
namespace legalization
{

bool legalizationCheck(const floorplan::Floorplan &floorplan, const placement::Placement &placement,
    const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist,
    const placement::Fences &fences)
{
    bool legality = checkAlignment(floorplan, placement, placementMapping, netlist) &&
                    checkBoundaries(floorplan, placement, placementMapping, netlist, fences) &&
                    checkCellOverlaps(placementMapping, netlist);
    return legality;
}

bool checkAlignment(const floorplan::Floorplan &floorplan, const placement::Placement &placement, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist)
{
    boost::geometry::index::rtree<std::pair<box, ophidian::floorplan::Row>, boost::geometry::index::rstar<16> > rows_rtree;
    for(auto row_it = floorplan.rowsRange().begin(); row_it != floorplan.rowsRange().end(); ++row_it)
    {
        auto row_origin = floorplan.origin(*row_it);
        auto row_upperRightCorner = floorplan.rowUpperRightCorner(*row_it);
        row_upperRightCorner.x(row_upperRightCorner.x() + row_origin.x());
        row_upperRightCorner.y(row_upperRightCorner.y() + row_origin.y());
        geometry::Box row_box(geometry::Point(units::unit_cast<double>(row_origin.x()), units::unit_cast<double>(row_origin.y())),
                              geometry::Point(units::unit_cast<double>(row_upperRightCorner.x()), units::unit_cast<double>(row_upperRightCorner.y())));
        rows_rtree.insert(std::make_pair(row_box, *row_it));
    }

    for (auto cell_it = netlist.begin(circuit::Cell()); cell_it != netlist.end(circuit::Cell()); ++cell_it)
    {
        if(placement.isFixed(*cell_it))
        {
            continue;
        }
        auto cellGeometry = placementMapping.geometry(*cell_it);
        for(auto cell_box : cellGeometry)
        {
            std::vector<std::pair<box, ophidian::floorplan::Row> > found_nodes;
            rows_rtree.query(boost::geometry::index::intersects(cell_box), std::back_inserter(found_nodes));
            if (found_nodes.empty())
            {
                std::cout << "cell not in line " << netlist.name(*cell_it) << " " << cell_box.min_corner().x() << ", " << cell_box.min_corner().y() << " -> " <<
                    cell_box.max_corner().x() << ", " << cell_box.max_corner().y() << std::endl;
                return false;
            }
            bool cell_aligned = false;
            for(auto pair_row_box : found_nodes)
            {
                auto row_origin = floorplan.origin(pair_row_box.second);
                auto site_width = units::unit_cast<double>(floorplan.siteUpperRightCorner(floorplan.site(pair_row_box.second)).x());
                auto x_cell_position = units::unit_cast<double>(placement.cellLocation(*cell_it).x());

                if ((placement.cellLocation(*cell_it).y() == row_origin.y()) && (std::abs(std::remainder(x_cell_position, site_width)) <= std::numeric_limits<double>::epsilon()))
                {
                    cell_aligned = true;
                    break;
                }
            }
            if(!cell_aligned)
            {
                std::cout << "cell " << netlist.name(*cell_it) << " " << cell_box.min_corner().x() << ", " << cell_box.min_corner().y() << " -> " <<
                    cell_box.max_corner().x() << ", " << cell_box.max_corner().y() << std::endl;
                std::cout << "cell movel? " <<  placement.isFixed(*cell_it) << std::endl;
                return false;
            }
        }
        //verify power rail alignment
        auto rowAlignment = placementMapping.alignment(*cell_it);
        if( rowAlignment != ophidian::placement::RowAlignment::NA)
        {
            auto cellLocation = placement.cellLocation(*cell_it);
            auto siteHeight = floorplan.siteUpperRightCorner(*floorplan.sitesRange().begin()).y();
            auto cellPlacedInOddRow = std::fmod((cellLocation.y()/siteHeight), 2.0);
            if(rowAlignment == ophidian::placement::RowAlignment::EVEN)
            {
                //Row alignmrnt = ODD
                if(cellPlacedInOddRow) {
                    std::cout << "cell " << netlist.name(*cell_it) << " " << cellLocation.x() << ", " << cellLocation.y() << " should be in an ODD row" << std::endl;
                    return false;
                }
            }
            else {
                //Row alignmrnt = EVEN
                if(!cellPlacedInOddRow) {
                    std::cout << "cell " << netlist.name(*cell_it) << " " << cellLocation.x() << ", " << cellLocation.y() << " should be in an EVEN row" << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}

bool checkBoundaries(const floorplan::Floorplan &floorplan, const placement::Placement &placement,
    const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist,
    const placement::Fences &fences)
{
    geometry::Point chipOrigin(units::unit_cast<double>(floorplan.chipOrigin().x()), units::unit_cast<double>(floorplan.chipOrigin().y()));
    geometry::Point chipUpperRightCorner(units::unit_cast<double>(floorplan.chipUpperRightCorner().x()), units::unit_cast<double>(floorplan.chipUpperRightCorner().y()));
    geometry::Box chip_area(chipOrigin, chipUpperRightCorner);
    for (auto cell_it = netlist.begin(circuit::Cell()); cell_it != netlist.end(circuit::Cell()); ++cell_it)
    {
        auto cellGeometry = placementMapping.geometry(*cell_it);
        bool hasFence = placement.cellHasFence(*cell_it);
        ophidian::util::MultiBox fenceGeometry;

        if (hasFence)
        {
            auto cellFence = placement.cellFence(*cell_it);
            fenceGeometry = fences.area(cellFence);
        }

        for(auto cell_box : cellGeometry)
        {
            if (!boost::geometry::within(cell_box, chip_area))
            {
                std::cout << "cell " << netlist.name(*cell_it) << " " << cell_box.min_corner().x() << ", " << cell_box.min_corner().y() << " -> " <<
                             cell_box.max_corner().x() << ", " << cell_box.max_corner().y() << std::endl;
                return false;
            }

            bool inFence = false;
            if (hasFence)
            {
                for (auto fence_box : fenceGeometry)
                {
                    inFence |= boost::geometry::covered_by(cell_box, fence_box);
                }
                if (!inFence)
                {
                    std::cout << netlist.name(*cell_it) << " not in fence" << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}


bool checkCellOverlaps(const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist)
{
    rtree cell_boxes_rtree;
    for (auto cell_it = netlist.begin(circuit::Cell()); cell_it != netlist.end(circuit::Cell()); ++cell_it)
    {
        auto cellGeometry = placementMapping.geometry(*cell_it);
        for(auto cell_box : cellGeometry)
        {
            std::vector<rtree_node> intersecting_nodes;
            cell_boxes_rtree.query( boost::geometry::index::within(cell_box), std::back_inserter(intersecting_nodes));
            cell_boxes_rtree.query( boost::geometry::index::overlaps(cell_box), std::back_inserter(intersecting_nodes));
            if (intersecting_nodes.empty())
            {
                cell_boxes_rtree.insert(std::make_pair(cell_box, *cell_it));
            }
            else {
                std::cout << "cell overlap " << netlist.name(*cell_it) << " " << cell_box.min_corner().x() << ", " << cell_box.min_corner().y() << " -> " <<
                    cell_box.max_corner().x() << ", " << cell_box.max_corner().y() << std::endl;
                std::cout << "intersecting boxes " << std::endl;
                for (auto node : intersecting_nodes)
                {
                    auto box = node.first;
                    std::cout << "cell " << netlist.name(node.second) << " " << box.min_corner().x() << ", " << box.min_corner().y() << " -> " <<
                        box.max_corner().x() << ", " << box.max_corner().y() << std::endl;
                }
                return false;
            }
        }
    }
    return true;
}



} // namespace legalization
} // namespace ophidian
