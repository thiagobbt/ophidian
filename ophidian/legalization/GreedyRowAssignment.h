#ifndef GREEDYROWASSIGNMENT_H
#define GREEDYROWASSIGNMENT_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/Subrows.h>

#include <ophidian/legalization/FenceRegionIsolation.h>

namespace ophidian {
namespace legalization {
class GreedyRowAssignment
{
public:
    using SubrowAssignment = std::pair<Subrow, double>;

    class SubrowAssignmentComparator
    {
    public:
        bool operator()(const SubrowAssignment & assignment1, const SubrowAssignment & assignment2) {
            return assignment1.second < assignment2.second;
        }
    };

    GreedyRowAssignment(design::Design & design);

    void assignCellsToRows();

private:
    void assignCellsToRows(util::MultiBox area, std::vector<circuit::Cell> & cells);

    design::Design & mDesign;

    Subrows mSubrows;
};
}
}

#endif // GREEDYROWASSIGNMENT_H
