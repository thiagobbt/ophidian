#ifndef UNCONSTRAINEDILPLEGALIZATION_H
#define UNCONSTRAINEDILPLEGALIZATION_H

#include <gurobi_c++.h>
#include <ophidian/placement/PlacementMapping.h>
#include <ophidian/floorplan/Floorplan.h>

namespace ophidian {
namespace legalization {
class UnconstrainedILPLegalization
{
public:
    UnconstrainedILPLegalization(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping);

    void legalizePlacement();

private:
    void addOverlapToObjective(GRBModel &model, GRBVar &objectiveVariable, std::string cell1Name, std::string cell2Name, GRBVar cell1Var, GRBVar cell2Var, double cell1Coordinate, double cell2Coordinate, double cell1Dimension, double cell2Dimension);

    const circuit::Netlist & netlist_;
    const floorplan::Floorplan & floorplan_;
    placement::Placement & placement_;
    const placement::PlacementMapping & placementMapping_;
};
}
}

#endif // UNCONSTRAINEDILPLEGALIZATION_H
