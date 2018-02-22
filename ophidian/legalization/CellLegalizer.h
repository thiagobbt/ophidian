#ifndef OPHIDIAN_LEGALIZATION_CELLLEGALIZER_H
#define OPHIDIAN_LEGALIZATION_CELLLEGALIZER_H

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <ophidian/circuit/Netlist.h>
#include <ophidian/design/Design.h>
#include <ophidian/geometry/Models.h>

#include <vector>

typedef ophidian::geometry::Point point;
typedef boost::geometry::model::box<point> Box;

namespace ophidian
{
namespace legalization
{

class CellLegalizer {
 public:
	CellLegalizer(design::Design & design);
	~CellLegalizer();

	// bool legalizeCell(circuit::Cell & targetCell, geometry::Point & targetPosition, std::vector<circuit::Cell> & legalizedCells, Box & legalizationRegion);
	bool legalizeCell(const circuit::Cell & targetCell, const geometry::Point & targetPosition, const Box & legalizationRegion);

 private:
	design::Design & mDesign;
};

} // namespace legalization
} // namespace ophidian

#endif
