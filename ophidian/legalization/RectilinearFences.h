#ifndef RECTILINEARFENCES_H
#define RECTILINEARFENCES_H

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {
class RectilinearFences
{
public:
    RectilinearFences(design::Design & design);

    void addBlocksToRectilinearFences();

    void addBlockToFence(placement::Fence fence);

    void eraseBlocks();

private:
    design::Design & mDesign;

    entity_system::Property<placement::Fence, std::vector<circuit::Cell>> mFenceBlocks;
};
}
}

#endif // RECTILINEARFENCES_H
