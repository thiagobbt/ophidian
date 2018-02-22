#ifndef OPHIDIAN_LEGALIZATION_CELLLEGALIZER_H
#define OPHIDIAN_LEGALIZATION_CELLLEGALIZER_H

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <ophidian/circuit/Netlist.h>
#include <ophidian/design/Design.h>
#include <ophidian/geometry/Models.h>

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

	void constructBaseTree();

	// bool legalizeCell(circuit::Cell & targetCell, geometry::Point & targetPosition, std::vector<circuit::Cell> & legalizedCells, Box & legalizationRegion);
	bool legalizeCell(const circuit::Cell & targetCell, const Point & targetPosition, const Box & legalizationRegion);

 private:
	design::Design & mDesign;
	RTree mBaseTree;
};

} // namespace legalization
} // namespace ophidian

#endif
