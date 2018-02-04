#ifndef ICCAD2017LEGALIZATION_H
#define ICCAD2017LEGALIZATION_H

#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/design/Design.h>
#include <ophidian/legalization/RtreeLegalization.h>
#include <ophidian/legalization/FenceRegionIsolation.h>
#include <ophidian/legalization/KDtreeLegalization.h>

namespace ophidian
{
namespace legalization
{
class iccad2017Legalization
{
public:
    iccad2017Legalization(ophidian::design::Design & design);

    void legalize();
    void kdtreeLegalization();
private:
    void flipCells();
    void legalizeFences();
    void allignCellsToNearestSite();
    void isolateFloorplan();
    void restoreFloorplan();

    ophidian::design::Design & mDesign;
    MultirowAbacus mMultirowAbacus;
    FenceRegionIsolation mFenceRegionIsolation;
    entity_system::Property<circuit::Cell, util::Location> mCellsInitialLocations;
    std::vector<ophidian::circuit::Cell> mFloorplanBlocks;
    unsigned mCountFloorplanBlocks;
    ophidian::geometry::Box mPlaceableArea;
};

} // namespace legalization
} // namespace ophidian
#endif // ICCAD2017LEGALIZATION_H
