#ifndef PHYSICALENGINELEGALIZATION_H
#define PHYSICALENGINELEGALIZATION_H

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {
class PhysicalEngineLegalization
{
    using RtreeNode = std::pair<geometry::Box, circuit::Cell>;
    using Rtree = boost::geometry::index::rtree<RtreeNode, boost::geometry::index::rstar<16> >;

public:
    PhysicalEngineLegalization(design::Design & design);

    void legalize();

    void initialize();

    void updateMovements(double timeStep);

    void checkOverlaps();

    void findMinimumTranslationVector(geometry::Box & box1, geometry::Box & box2, geometry::Point & mtv);

private:
    design::Design & mDesign;

    entity_system::Property<circuit::Cell, geometry::Point> mCellVelocities;
    entity_system::Property<circuit::Cell, double> mCellMasses;
    entity_system::Property<circuit::Cell, geometry::Point> mCellForces;
    entity_system::Property<circuit::Cell, geometry::Point> mCellImpulses;

    Rtree mRtree;
};
}
}

#endif // PHYSICALENGINELEGALIZATION_H
