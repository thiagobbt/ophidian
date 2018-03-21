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

    void addFenceBlocks(placement::Fence fence, bool align = false);

    void restoreAllFenceCells();

    void restoreFenceCells(placement::Fence fence);

    void eraseFenceBlocks(placement::Fence fence);

    void fixCellsInFences(bool fix);

    void fixCellsInFence(placement::Fence fence, bool fix);
private:

    design::Design & mDesign;

    unsigned mContBox;
    entity_system::Property<placement::Fence, std::vector<circuit::Cell>> mFenceBlocks;
};
}
}


#endif // FENCEREGIONISOLATION_H
