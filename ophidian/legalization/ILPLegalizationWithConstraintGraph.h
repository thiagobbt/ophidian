#ifndef ILPLEGALIZATIONWITHCONSTRAINTGRAPH_H
#define ILPLEGALIZATIONWITHCONSTRAINTGRAPH_H

#include <gurobi_c++.h>

#include <ophidian/legalization/ConstraintGraph.h>


namespace ophidian {
namespace legalization {
class ILPLegalizationWithConstraintGraph
{
public:
    ILPLegalizationWithConstraintGraph(design::Design & design);

    void legalize(const std::vector<circuit::Cell> & cells, const geometry::Box &legalizationArea);

    void writeGraphFile(const std::vector<circuit::Cell> & cells);

private:
    design::Design & mDesign;

    ConstraintGraph<LeftComparator> mHorizontalConstraintGraph;
    ConstraintGraph<BelowComparator> mVerticalConstraintGraph;
};
}
}

#endif // ILPLEGALIZATIONWITHCONSTRAINTGRAPH_H
