#ifndef BINDECOMPOSITION_H
#define BINDECOMPOSITION_H

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {

class Bin : public entity_system::EntityBase {
public:
    using entity_system::EntityBase::EntityBase;
};

class BinDecomposition
{
    using RtreeNode = std::pair<geometry::Box, Bin>;
    using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16> >;

    using BinsIterator = entity_system::EntitySystem<Bin>::const_iterator;

public:
    BinDecomposition(design::Design & design);

    void decomposeCircuitInBins(util::MultiBox area, std::vector<circuit::Cell> & cells, unsigned binSizeInRows = 10);

    std::size_t size(Bin);

    geometry::Box box(Bin bin) {
        return mBinsBoxes[bin];
    }

    std::vector<circuit::Cell> & cells(Bin bin) {
        return mBinsCells[bin];
    }

    ophidian::util::Range<BinsIterator> range(Bin) const;
private:
    design::Design & mDesign;

    entity_system::EntitySystem<Bin> mBins;
    entity_system::Property<Bin, std::vector<circuit::Cell>> mBinsCells;
    entity_system::Property<Bin, geometry::Box> mBinsBoxes;

    Rtree mBinsRtree;
};
}
}

#endif // BINDECOMPOSITION_H
