#ifndef KDTREELEGALIZATION_H
#define KDTREELEGALIZATION_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/Subrows.h>
#include <ophidian/geometry/Models.h>
#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationKDtree.h>

namespace ophidian {
namespace legalization {

using box = geometry::Box;
using rtree_node = std::pair<box, circuit::Cell>;
using rtree = boost::geometry::index::rtree<rtree_node, boost::geometry::index::rstar<16> >;

class KDtreeLegalization
{
public:
    KDtreeLegalization(design::Design & design);
    void build(ophidian::geometry::Box legalizationArea, unsigned int i);
    void legalize();
    void density() const;

private:
    void allignCellsToNearestSite();
    void removeMacroblocksOverlaps();

    design::Design & mDesign;
    LegalizationKDtree mKDTree;
    util::MultiBox mPlaceableArea;
    std::vector<std::shared_ptr<ophidian::circuit::Cell>> mAncients;
    std::vector<LegalizationKDtree::Partition> mPartitions;
    entity_system::Property<circuit::Cell, util::Location> mCellsInitialLocations;
};
}
}

#endif // KDTREELEGALIZATION_H
