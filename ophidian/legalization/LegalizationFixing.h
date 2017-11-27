#ifndef LEGALIZATIONFIXING_H
#define LEGALIZATIONFIXING_H

#include <ophidian/legalization/CellShifting.h>

namespace ophidian {
namespace legalization {
class LegalizationFixing
{
public:
    LegalizationFixing(design::Design & design);

    void fixLegalization(util::MultiBox area, std::vector<circuit::Cell> &cells);

private:
    design::Design & mDesign;

    entity_system::Property<circuit::Cell, util::Location> mInitialLocations;

    Subrows mSubrows;
};
}
}


#endif // LEGALIZATIONFIXING_H
