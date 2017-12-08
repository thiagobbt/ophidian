#ifndef ROWASSIGNMENTFIXTURE_H
#define ROWASSIGNMENTFIXTURE_H

#include <ophidian/legalization/RowAssignment.h>
#include <ophidian/legalization/MixedRowAssignment.h>

class RowAssignmentFixture : public ophidian::legalization::RowAssignment
{
public:
    RowAssignmentFixture(ophidian::design::Design & design);

    void assignCellsToRows();

    bool checkSubrowsCapacities();

private:
    ophidian::entity_system::Property<ophidian::legalization::Subrow, std::vector<ophidian::circuit::Cell>> mSubrowsCells;
};

#endif // ROWASSIGNMENTFIXTURE_H
