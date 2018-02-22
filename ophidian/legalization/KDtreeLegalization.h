#ifndef KDTREELEGALIZATION_H
#define KDTREELEGALIZATION_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/Subrows.h>
#include <ophidian/geometry/Models.h>
#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationKDtree.h>

namespace ophidian {
namespace legalization {


class KDtreeLegalization
{
public:
    KDtreeLegalization(design::Design & design);
    void build(ophidian::geometry::Box legalizationArea, unsigned int i);
    void legalize();
    void density() const;

private:
    void allignCellsToNearestSite();

    design::Design & mDesign;
    LegalizationKDtree mKDTree;
    util::MultiBox mPlaceableArea;
    std::vector<std::shared_ptr<ophidian::circuit::Cell>> mAncients;
    std::vector<LegalizationKDtree::Partition> mPartitions;
};
}
}

#endif // KDTREELEGALIZATION_H
