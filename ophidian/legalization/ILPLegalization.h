#ifndef ILPLEGALIZATION_H
#define ILPLEGALIZATION_H

#include <gurobi_c++.h>
#include <ophidian/placement/PlacementMapping.h>
#include <ophidian/floorplan/Floorplan.h>

namespace ophidian {
namespace legalization {
class ILPLegalization
{
public:
    ILPLegalization(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping);

    void legalizePlacement();

    void legalize(const std::vector<circuit::Cell> &cells, const geometry::Box & legalizationArea);

private:
    void addOverlapConstraint(GRBModel &model, std::string cell1Name, std::string cell2Name, GRBVar cell1Var, GRBVar cell2Var, double cell1Coordinate, double cell2Coordinate, double cell1Dimension, double cell2Dimension);

    const circuit::Netlist & netlist_;
    const floorplan::Floorplan & floorplan_;
    placement::Placement & placement_;
    const placement::PlacementMapping & placementMapping_;
};
}
}

#endif // ILPLEGALIZATION_H
