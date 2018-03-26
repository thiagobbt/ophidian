#ifndef OPHIDIAN_LEGALIZATION_CELLLEGALIZER_H
#define OPHIDIAN_LEGALIZATION_CELLLEGALIZER_H

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <ophidian/circuit/Netlist.h>
#include <ophidian/design/Design.h>
#include <ophidian/geometry/Models.h>
// #include <ophidian/legalization/FenceRegionIsolation.h>
// #include <ophidian/legalization/RectilinearFences.h>

#include <vector>

typedef ophidian::geometry::Point Point;
typedef boost::geometry::model::box<ophidian::geometry::Point> Box;
typedef std::pair<Box, ophidian::circuit::Cell> RNode;
typedef boost::geometry::index::rtree<RNode, boost::geometry::index::rstar<16>> RTree;

namespace ophidian
{
namespace legalization
{

class CellLegalizer {
 public:
	CellLegalizer(design::Design & design);
	~CellLegalizer();

	void buildRtree(const std::vector<circuit::Cell> legalizedCells);
	long long legalizeCell(const circuit::Cell & targetCell, const geometry::Point & targetPosition, const std::vector<circuit::Cell> legalizedCells, const Box & legalizationRegion, bool estimateOnly = false);
	long long legalizeCell(const circuit::Cell & targetCell, const geometry::Point & targetPosition, const Box & legalizationRegion, bool estimateOnly = false);

 private:
	design::Design & mDesign;
	RTree mTree;
	// FenceRegionIsolation mFenceRegionIsolation;
	// RectilinearFences mRectilinearFences;
};

} // namespace legalization
} // namespace ophidian

#endif
