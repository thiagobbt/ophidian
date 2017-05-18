#include "LegalizationCheck.h"

namespace ophidian {
namespace legalization {

bool legalizationCheck(const floorplan::Floorplan &floorplan, const placement::Placement &placement, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist)
{
   bool legality = checkAlignment(floorplan, placement, placementMapping, netlist) && checkBoundaries(floorplan, placementMapping, netlist) && checkCellOverlaps(placementMapping, netlist);
   return legality;
}

bool checkAlignment(const floorplan::Floorplan &floorplan, const placement::Placement &placement, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist)
{
    boost::geometry::index::rtree<std::pair<box, ophidian::floorplan::Row>, boost::geometry::index::rstar<16>> rows_rtree;
    for(auto row_it = floorplan.rowsRange().begin(); row_it != floorplan.rowsRange().end(); ++row_it){
        auto row_origin = floorplan.origin(*row_it);
        auto row_upperRightCorner = floorplan.rowUpperRightCorner(*row_it);
        row_upperRightCorner.x(row_upperRightCorner.x() + row_origin.x());
        row_upperRightCorner.y(row_upperRightCorner.y() + row_origin.y());
        geometry::Box row_box(geometry::Point(units::unit_cast<double>(row_origin.x()), units::unit_cast<double>(row_origin.y())),
                              geometry::Point(units::unit_cast<double>(row_upperRightCorner.x()), units::unit_cast<double>(row_upperRightCorner.y())));
        rows_rtree.insert(std::make_pair(row_box, *row_it));
    }

    for (auto cell_it = netlist.begin(circuit::Cell()); cell_it != netlist.end(circuit::Cell()); ++cell_it) {
        auto cellGeometry = placementMapping.geometry(*cell_it);
        for(auto cell_box : cellGeometry){
            std::vector<std::pair<box, ophidian::floorplan::Row>> found_nodes;
            rows_rtree.query(boost::geometry::index::intersects(cell_box), std::back_inserter(found_nodes));
            if (found_nodes.empty()) {
                return false;
            }
            bool cell_aligned = false;
            for(auto pair_row_box : found_nodes){
                auto row_origin = floorplan.origin(pair_row_box.second);
                auto site_width = units::unit_cast<double>(floorplan.siteUpperRightCorner(floorplan.site(pair_row_box.second)).x());
                auto x_cell_position = units::unit_cast<double>(placement.cellLocation(*cell_it).x());

                if ((placement.cellLocation(*cell_it).y() == row_origin.y()) && (std::abs(std::remainder(x_cell_position, site_width)) <= std::numeric_limits<double>::epsilon())) {
                    cell_aligned = true;
                    break;
                }
            }
            if(!cell_aligned){
                return false;
            }
        }
    }
    return true;
}

bool checkBoundaries(const floorplan::Floorplan &floorplan, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist)
{
    geometry::Point chipOrigin(units::unit_cast<double>(floorplan.chipOrigin().x()), units::unit_cast<double>(floorplan.chipOrigin().y()));
    geometry::Point chipUpperRightCorner(units::unit_cast<double>(floorplan.chipUpperRightCorner().x()), units::unit_cast<double>(floorplan.chipUpperRightCorner().y()));
    geometry::Box chip_area(chipOrigin, chipUpperRightCorner);
    for (auto cell_it = netlist.begin(circuit::Cell()); cell_it != netlist.end(circuit::Cell()); ++cell_it) {
        auto cellGeometry = placementMapping.geometry(*cell_it);
        for(auto cell_box : cellGeometry){
            if (!boost::geometry::within(cell_box, chip_area)) {
                return false;
            }
        }
    }
    return true;
}


bool checkCellOverlaps(const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist)
{
    rtree cell_boxes_rtree;
    for (auto cell_it = netlist.begin(circuit::Cell()); cell_it != netlist.end(circuit::Cell()); ++cell_it) {
        auto cellGeometry = placementMapping.geometry(*cell_it);
        for(auto cell_box : cellGeometry){
            std::vector<rtree_node> intersecting_nodes;
            cell_boxes_rtree.query( boost::geometry::index::within(cell_box), std::back_inserter(intersecting_nodes));
            cell_boxes_rtree.query( boost::geometry::index::overlaps(cell_box), std::back_inserter(intersecting_nodes));
            if (intersecting_nodes.empty()) {
                cell_boxes_rtree.insert(std::make_pair(cell_box, *cell_it));
            } else {
                return false;
            }
        }
    }
    return true;
}



}
}
