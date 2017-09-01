#ifndef RTREELEGALIZATION_H
#define RTREELEGALIZATION_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/Subrows.h>

namespace ophidian {
namespace legalization {
class RtreeLegalization
{
public:
    using RtreeNode = std::pair<geometry::Box, circuit::Cell>;
    using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16> >;

    enum OverlapDirection {LEFT, RIGHT, ANY};

    class Overlap {
    public:
        circuit::Cell firstCell;
        geometry::Box firstCellBox;
        circuit::Cell secondCell;
        geometry::Box secondCellBox;
        OverlapDirection direction;

        bool operator<(const Overlap & overlap) const {
            if (this->direction == overlap.direction) {
                if (this->direction == OverlapDirection::LEFT) {
                    return this->secondCellBox.min_corner().x() > overlap.secondCellBox.min_corner().x();
                } else {
                    return this->secondCellBox.min_corner().x() < overlap.secondCellBox.min_corner().x();
                }
            } else {
                return (this->direction == OverlapDirection::LEFT && overlap.direction == OverlapDirection::RIGHT);
            }
        }

        bool operator==(const Overlap & overlap) const {
            return this->secondCell == overlap.secondCell;
        }
    };

    RtreeLegalization(design::Design & design);

    void legalizePlacement(std::vector<circuit::Cell> & cells, ophidian::util::MultiBox & legalizationArea);

    void legalizeCell(circuit::Cell cell);

    void buildRtree(std::vector<circuit::Cell> & cells, util::MultiBox legalizationArea);
private:
    bool legalizeCell(circuit::Cell cell, util::Location targetLocation);

    void findOverlaps(circuit::Cell cell, util::Location targetLocation, std::set<Overlap> &overlaps, OverlapDirection previousDirection);

    design::Design & mDesign;
    Rtree mCellsRtree;

    entity_system::Property<circuit::Cell, util::Location> mCellsInitialLocations;

    entity_system::Property<circuit::Cell, Subrow> mCell2Subrow;

    Subrows mSubrows;
};
}
}


#endif // RTREELEGALIZATION_H
