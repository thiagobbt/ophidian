#ifndef SUBROWS_H
#define SUBROWS_H

#include <ophidian/floorplan/Floorplan.h>
#include <ophidian/placement/PlacementMapping.h>

namespace ophidian {
namespace legalization {

class Subrow : public entity_system::EntityBase
{
    public:
        using entity_system::EntityBase::EntityBase;
};

using RtreeNode = std::pair<geometry::Box, Subrow>;
using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16>>;

class Subrows
{
public:
    using SubrowsIterator = entity_system::EntitySystem<Subrow>::const_iterator;

    Subrows(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping);

    void createSubrows();

    ophidian::util::Range<SubrowsIterator> range(Subrow) const;

    void findClosestSubrows(unsigned numberOfSubrows, util::Location point, std::vector<Subrow> & subrows) const;

    ophidian::util::micrometer_t capacity(Subrow subrow) const;

    void capacity(Subrow subrow, ophidian::util::micrometer_t capacity);

    util::Location origin(Subrow subrow) const;

    util::Location upperCorner(Subrow subrow) const;

    template <typename Value>
    entity_system::Property<Subrow, Value> makeProperty(Subrow)
    const {
        return entity_system::Property<Subrow, Value>(subrows_);
    }

private:
    const circuit::Netlist & netlist_;
    const floorplan::Floorplan & floorplan_;
    placement::Placement & placement_;
    const placement::PlacementMapping & placementMapping_;

    entity_system::EntitySystem<Subrow> subrows_;
    entity_system::Property<Subrow, util::Location> subrowOrigins_;
    entity_system::Property<Subrow, util::Location> subrowUpperCorners_;
    entity_system::Property<Subrow, ophidian::util::micrometer_t> subrowCapacities_;

    Rtree subrowsRtree_;
};
}
}


#endif // SUBROWS_H
