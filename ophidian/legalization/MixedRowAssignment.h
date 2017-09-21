#ifndef MIXEDROWASSIGNMENT_H
#define MIXEDROWASSIGNMENT_H

#include <unordered_map>

#include <gurobi_c++.h>

#include <ophidian/legalization/Subrows.h>

#include <ophidian/design/Design.h>

#include <ophidian/legalization/FenceRegionIsolation.h>

#include <ophidian/legalization/RowAssignment.h>

namespace ophidian {
namespace legalization {

class MixedRowAssignment
{
public:
    using AssignmentPair = std::pair<GRBVar, Subrow>;

    MixedRowAssignment(design::Design & design);

    void assignCellsToRows();
protected:
    void assignCellsToRows(util::MultiBox area, std::vector<circuit::Cell> & cells);

    void assignCellsToRows(util::MultiBox area, std::vector<circuit::Cell> & cells, entity_system::Property<circuit::Cell, char> & cellVariableTypes);

    void sliceCells(std::vector<circuit::Cell> & cells);

    void addAssignmentVariables(CellSlice cellSlice, entity_system::Property<Subrow, GRBLinExpr> & capacityConstraints, entity_system::Property<CellSlice, std::vector<AssignmentPair> > &assignmentVariables, std::string cellName, util::Location cellLocation, GRBModel & model, double cellWidth, GRBLinExpr & objectiveFunction, char variableType);

    bool sameAlignment(CellSlice cellSlice, Subrow subrow);

    design::Design & mDesign;

    Subrows mSubrows;

    entity_system::EntitySystem<CellSlice> mCellSlices;
    entity_system::Property<circuit::Cell, std::vector<CellSlice>> mCircuitCellsSlices;
    entity_system::Property<CellSlice, circuit::Cell> mSlice2Cell;
    entity_system::Property<CellSlice, placement::RowAlignment> mSliceAlignment;

    entity_system::Property<CellSlice, Subrow> mSliceAssignment;
};
}
}

#endif // MIXEDROWASSIGNMENT_H
