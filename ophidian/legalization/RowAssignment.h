#ifndef ROWASSIGNMENT_H
#define ROWASSIGNMENT_H

#include <unordered_map>

#include <gurobi_c++.h>

#include <ophidian/legalization/Subrows.h>

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {
class CellSlice : public entity_system::EntityBase
{
public:
    using entity_system::EntityBase::EntityBase;
};

class RowAssignment
{
public:
    using AssignmentPair = std::pair<GRBVar, Subrow>;

    RowAssignment(design::Design & design);

    void assignCellsToRows(util::MultiBox area);

protected:
    void sliceCells();

    void addAssignmentVariables(CellSlice cellSlice, entity_system::Property<Subrow, GRBLinExpr> & capacityConstraints, entity_system::Property<CellSlice, std::vector<AssignmentPair> > &assignmentVariables, std::string cellName, util::Location cellLocation, GRBModel & model, double cellWidth, GRBLinExpr & objectiveFunction);

    design::Design & mDesign;

    Subrows mSubrows;

    entity_system::EntitySystem<CellSlice> mCellSlices;
    entity_system::Property<circuit::Cell, std::vector<CellSlice>> mCircuitCellsSlices;
    entity_system::Property<CellSlice, circuit::Cell> mSlice2Cell;
};
}
}

#endif // ROWASSIGNMENT_H
