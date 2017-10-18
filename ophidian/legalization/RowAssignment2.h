#ifndef ROWASSIGNMENT2_H
#define ROWASSIGNMENT2_H

#include <gurobi_c++.h>

#include <ophidian/legalization/Subrows.h>
#include <ophidian/legalization/FenceRegionIsolation.h>
#include <ophidian/legalization/BinDecomposition.h>

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {
class CellSlice : public entity_system::EntityBase
{
public:
    using entity_system::EntityBase::EntityBase;
};

class RowAssignment2
{
public:
    class CellSlicePairComparator
    {
    public:
        bool operator()(const std::pair<CellSlice, util::micrometer_t> & cellPair1, const std::pair<CellSlice, util::micrometer_t> & cellPair2) {
            return cellPair1.second < cellPair2.second;
        }
    };

    using AssignmentPair = std::pair<GRBVar, Subrow>;

    RowAssignment2(design::Design & design);

    void assignCellsToRows();
protected:
    void assignCellsToRows(util::MultiBox area, std::vector<circuit::Cell> & cells);
    void createAndSolveGurobiModel(std::vector<circuit::Cell> &cells);

    void sliceCells(std::vector<circuit::Cell> & cells);

    void addAssignmentVariables(CellSlice cellSlice, entity_system::Property<Subrow, GRBLinExpr> & capacityConstraints, entity_system::Property<CellSlice, std::vector<AssignmentPair> > &assignmentVariables, std::string cellName, util::Location cellLocation, GRBModel & model, double cellWidth, GRBLinExpr & objectiveFunction, const entity_system::Property<Subrow, std::vector<std::pair<CellSlice, util::micrometer_t>>> & subrowsSlices);

    bool sameAlignment(CellSlice cellSlice, Subrow subrow);

    design::Design & mDesign;

    Subrows mSubrows;

    entity_system::EntitySystem<CellSlice> mCellSlices;
    entity_system::Property<circuit::Cell, std::vector<CellSlice>> mCircuitCellsSlices;
    entity_system::Property<CellSlice, circuit::Cell> mSlice2Cell;
    entity_system::Property<CellSlice, placement::RowAlignment> mSliceAlignment;
};
}
}

#endif // ROWASSIGNMENT2_H
