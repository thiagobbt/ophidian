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

private:
    template <class ComparatorType1, class ComparatorType2>
    void adjustConstraintGraph(ConstraintGraph<ComparatorType1> & horizontalGraph, ConstraintGraph<ComparatorType2> & verticalGraph, util::micrometer_t horizontalMin, util::micrometer_t horizontalMax, util::micrometer_t verticalMin, util::micrometer_t verticalMax) {
        unsigned iterationIndex = 0;
        std::cout << "horizontal graph worst slack " << horizontalGraph.worstSlack() << std::endl;
        std::cout << "vertical graph worst slack " << verticalGraph.worstSlack() << std::endl;
        while ((!horizontalGraph.isFeasible() || !verticalGraph.isFeasible()) && iterationIndex < 100) {
            if (!horizontalGraph.isFeasible()) {
                horizontalGraph.adjustGraph(verticalGraph, horizontalMin, horizontalMax, verticalMin, verticalMax);
            } else {
                verticalGraph.adjustGraph(horizontalGraph, verticalMin, verticalMax, horizontalMin, horizontalMax);
            }

            std::cout << "horizontal graph worst slack " << horizontalGraph.worstSlack() << std::endl;
            std::cout << "vertical graph worst slack " << verticalGraph.worstSlack() << std::endl;

            iterationIndex++;
        }
    }

    design::Design & mDesign;

    ConstraintGraph<LeftComparator> mHorizontalConstraintGraph;
    ConstraintGraph<BelowComparator> mVerticalConstraintGraph;
};
}
}

#endif // ILPLEGALIZATIONWITHCONSTRAINTGRAPH_H
