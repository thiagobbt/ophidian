#ifndef LEGALIZATIONCHECK_H
#define LEGALIZATIONCHECK_H

#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "ophidian/floorplan/Floorplan.h"
#include "ophidian/placement/Placement.h"
#include "ophidian/circuit/Netlist.h"
#include "ophidian/placement/PlacementMapping.h"
#include "ophidian/util/Units.h"

namespace ophidian
{
namespace legalization
{

using point = geometry::Point;
using box = geometry::Box;
using rtree_node = std::pair<box, circuit::Cell>;
using rtree = boost::geometry::index::rtree<rtree_node, boost::geometry::index::rstar<16> >;

bool legalizationCheck(const floorplan::Floorplan &floorplan, const placement::Placement &placement, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist, const placement::Fences &fences);

bool checkAlignment(const floorplan::Floorplan &floorplan, const placement::Placement &placement, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist);
bool checkBoundaries(const floorplan::Floorplan &floorplan, const placement::Placement &placement, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist, const placement::Fences &fences);
bool checkCellOverlaps(const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist);

void getUnaligned(const floorplan::Floorplan &floorplan, const placement::Placement &placement, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist, std::vector<circuit::Cell> & unaligned_cells);
void getOutsideBoundaries(const floorplan::Floorplan &floorplan, const placement::Placement &placement, const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist, const placement::Fences &fences, std::vector<circuit::Cell> & cells_outside_boundaries);
void getOverlapping(const placement::PlacementMapping &placementMapping, const circuit::Netlist &netlist, std::vector<circuit::Cell> & overlapping_cells);

} // namespace legalization
} // namespace ophidian

#endif // LEGALIZATIONCHECK_H
