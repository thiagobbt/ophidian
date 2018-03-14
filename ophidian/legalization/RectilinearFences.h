#ifndef RECTILINEARFENCES_H
#define RECTILINEARFENCES_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/FenceRegionIsolation.h>
#include <ophidian/legalization/Subrows.h>

namespace ophidian {
namespace legalization {
class RectilinearFences
{
    using RtreeNode = std::pair<geometry::Box, Subrow>;
    using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16> >;

public:
    RectilinearFences(design::Design & design);

    void addBlocksToRectilinearFences();

    void addBlockToFence(placement::Fence fence);

    void eraseBlocks();

private:
    void mergeSubrows(Subrow subrow1, Subrow subrow2);

    design::Design & mDesign;

    entity_system::Property<placement::Fence, std::vector<circuit::Cell>> mFenceBlocks;

    FenceRegionIsolation mIsolateFences;

//    entity_system::Property<Subrow, circuit::Cell> mSubrow2Block;
//    entity_system::Property<circuit::Cell, std::vector<Subrow>> mBlock2Subrows;
};
}
}

#endif // RECTILINEARFENCES_H
