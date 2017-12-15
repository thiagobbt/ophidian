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
    void adjustConstraintGraph(ConstraintGraph<ComparatorType1> & graph1, ConstraintGraph<ComparatorType2> & graph2, util::micrometer_t min, util::micrometer_t max, util::micrometer_t orthogonalMin, util::micrometer_t orthogonalMax) {
        unsigned iterationIndex = 0;
        std::cout << "graph 1 worst slack " << graph1.worstSlack() << std::endl;
        std::cout << "graph 2 worst slack " << graph2.worstSlack() << std::endl;
        while (!graph1.isFeasible() && iterationIndex < 100) {
            graph1.adjustGraph(graph2, min, max, orthogonalMin, orthogonalMax);

            std::cout << "graph 1 worst slack " << graph1.worstSlack() << std::endl;
            std::cout << "graph 2 worst slack " << graph2.worstSlack() << std::endl;

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
