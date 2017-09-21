#ifndef ICCAD2017LEGALIZATION_H
#define ICCAD2017LEGALIZATION_H

#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/design/Design.h>
#include <ophidian/legalization/RtreeLegalization.h>
#include <ophidian/legalization/FenceRegionIsolation.h>

namespace ophidian
{
namespace legalization
{
class iccad2017Legalization
{
public:
    iccad2017Legalization(ophidian::design::Design & design);

    void legalize();
private:
    void flipCells();

    ophidian::design::Design & mDesign;
    MultirowAbacus mMultirowAbacus;

    FenceRegionIsolation mFenceRegionIsolation;

    entity_system::Property<circuit::Cell, util::Location> mCellsInitialLocations;
};

} // namespace legalization
} // namespace ophidian
#endif // ICCAD2017LEGALIZATION_H
