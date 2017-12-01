#ifndef KDTREELEGALIZATION_H
#define KDTREELEGALIZATION_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/Subrows.h>
#include <ophidian/geometry/Models.h>

#include <ophidian/util/KDtree.h>

namespace ophidian {
namespace legalization {


class KDtreeLegalization
{
public:
    KDtreeLegalization(design::Design & design);
    void build();
    void splitTree(unsigned int k);
    const std::vector<std::shared_ptr<ophidian::circuit::Cell>> ancients() const;
    const std::vector<std::pair<std::vector<std::shared_ptr<ophidian::circuit::Cell>>, geometry::Box>> subTrees() const;
    void density() const;

private:
    design::Design & mDesign;
    util::KDtree<ophidian::circuit::Cell> mKDTree;
    std::vector<std::shared_ptr<ophidian::circuit::Cell>> mAncients;
    std::vector<std::pair<std::vector<std::shared_ptr<ophidian::circuit::Cell>>, geometry::Box>> mSubTrees;
};
}
}


#endif // KDTREELEGALIZATION_H
