#ifndef FENCEREGIONISOLATION_H
#define FENCEREGIONISOLATION_H

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {
class FenceRegionIsolation
{
public:
    FenceRegionIsolation(design::Design & design);

    void isolateAllFenceCells();

    void isolateFenceCells(placement::Fence fence);

    void restoreAllFenceCells();

    void restoreFenceCells(placement::Fence fence);

private:
    void fixCellsInFence(placement::Fence fence, bool fix);

    design::Design & mDesign;

    unsigned mContBox;
    entity_system::Property<placement::Fence, std::vector<circuit::Cell>> mFenceBlocks;
};
}
}


#endif // FENCEREGIONISOLATION_H
