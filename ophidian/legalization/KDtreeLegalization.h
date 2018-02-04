#ifndef KDTREELEGALIZATION_H
#define KDTREELEGALIZATION_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/Subrows.h>
#include <ophidian/geometry/Models.h>
#include <ophidian/legalization/MultirowAbacus.h>

#include <ophidian/util/KDtree.h>

namespace ophidian {
namespace legalization {


class KDtreeLegalization
{
public:
    KDtreeLegalization(design::Design & design);
    void build(ophidian::geometry::Box legalizationArea);
    void legalize();
    void density() const;

private:
    void splitTree(unsigned int k);
    void allignCellsToNearestSite();

    design::Design & mDesign;
    util::KDtree<ophidian::circuit::Cell> mKDTree;
    util::MultiBox mPlaceableArea;
    std::vector<std::shared_ptr<ophidian::circuit::Cell>> mAncients;
    std::vector<std::pair<std::vector<std::shared_ptr<ophidian::circuit::Cell>>, geometry::Box>> mSubTrees;
};
}
}


#endif // KDTREELEGALIZATION_H
