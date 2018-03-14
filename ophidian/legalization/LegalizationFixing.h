#ifndef LEGALIZATIONFIXING_H
#define LEGALIZATIONFIXING_H

#include <ophidian/legalization/ConstraintGraph.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/legalization/Subrows.h>
#include <ophidian/legalization/CellLegalizer.h>

namespace ophidian {
namespace legalization {
class LegalizationFixing
{
    class CellPairComparator
    {
public:
        bool operator()(const std::pair<circuit::Cell, double> & cellPair1, const std::pair<circuit::Cell, double> & cellPair2) {
            return cellPair1.second > cellPair2.second;
        }
    };

public:
    LegalizationFixing(design::Design & design);

    void fixLegalization(const std::vector<circuit::Cell> &cells, geometry::Box area);

    void placeCellsOverOtherCells(std::vector<std::pair<circuit::Cell, double> > &unlegalizedCells, std::vector<circuit::Cell> & legalizedCells, geometry::Box area, std::vector<std::pair<circuit::Cell, double> > & newUnlegalizedCells);

    void placeCellsInSubrows(std::vector<std::pair<circuit::Cell, double>> & unlegalizedCells, std::vector<circuit::Cell> & legalizedCells, geometry::Box area, std::vector<std::pair<circuit::Cell, double> > & newUnlegalizedCells);

private:
    design::Design & mDesign;

    entity_system::Property<circuit::Cell, util::Location> mInitialLocations;
    entity_system::Property<circuit::Cell, bool> mInitialFixed;

    CellLegalizer mLegalizer;
};
}
}


#endif // LEGALIZATIONFIXING_H
