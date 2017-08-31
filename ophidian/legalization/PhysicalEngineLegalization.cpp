#include "PhysicalEngineLegalization.h"

namespace ophidian {
namespace legalization {
PhysicalEngineLegalization::PhysicalEngineLegalization(design::Design &design)
    : mDesign(design),
      mCellVelocities(design.netlist().makeProperty<geometry::Point>(circuit::Cell())),
      mCellMasses(design.netlist().makeProperty<double>(circuit::Cell())),
      mCellForces(design.netlist().makeProperty<geometry::Point>(circuit::Cell())),
      mCellImpulses(design.netlist().makeProperty<geometry::Point>(circuit::Cell())) {

}

void PhysicalEngineLegalization::legalize()
{
    unsigned numberOfIterations = 100;
    double timeStep = 0.02;
    for (unsigned iteration = 0; iteration < numberOfIterations; iteration) {
        updateMovements(timeStep);
        checkOverlaps();
    }
}

void PhysicalEngineLegalization::initialize()
{
    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); cellIt++) {
        mCellVelocities[*cellIt].x(0.0);
        mCellVelocities[*cellIt].y(0.0);

        mCellMasses[*cellIt] = 1.0;

        mCellForces[*cellIt].x(0.0);
        mCellForces[*cellIt].y(0.0);

        mCellImpulses[*cellIt].x(0.0);
        mCellImpulses[*cellIt].y(0.0);

        auto cellGeometry = mDesign.placementMapping().geometry(*cellIt);
        for (auto cellBox : cellGeometry) {
            mRtree.insert(RtreeNode(cellBox, *cellIt));
        }
    }
}

void PhysicalEngineLegalization::updateMovements(double timeStep)
{
    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); cellIt++) {
        mCellVelocities[*cellIt].x(mCellVelocities[*cellIt].x() + (mCellForces[*cellIt].x()*timeStep/mCellMasses[*cellIt]));
        mCellVelocities[*cellIt].x(mCellVelocities[*cellIt].x() + (mCellImpulses[*cellIt].x()/mCellMasses[*cellIt]));
        mCellVelocities[*cellIt].y(mCellVelocities[*cellIt].y() + (mCellForces[*cellIt].y()*timeStep/mCellMasses[*cellIt]));
        mCellVelocities[*cellIt].y(mCellVelocities[*cellIt].y() + (mCellImpulses[*cellIt].x()/mCellMasses[*cellIt]));

        geometry::Point translation(mCellVelocities[*cellIt].x() * timeStep, mCellVelocities[*cellIt].y() * timeStep);

        mCellForces[*cellIt].x(0.0);
        mCellForces[*cellIt].y(0.0);

        mCellImpulses[*cellIt].x(0.0);
        mCellImpulses[*cellIt].y(0.0);

        auto cellLocation = mDesign.placement().cellLocation(*cellIt);
        mDesign.placement().placeCell(*cellIt, util::Location(cellLocation.toPoint().x() + translation.x(), cellLocation.toPoint().y() + translation.y()));
    }
}

void PhysicalEngineLegalization::checkOverlaps()
{
    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); cellIt++) {
        auto cellGeometry = mDesign.placementMapping().geometry(*cellIt);
        auto cellBox = *cellGeometry.begin();
        std::vector<RtreeNode> intersectingCells;
        mRtree.query(boost::geometry::index::intersects(cellBox), std::back_inserter(intersectingCells));

        for (auto node : intersectingCells) {
            geometry::Point mtv;
            findMinimumTranslationVector(cellBox, node.first, mtv);

            geometry::Box newBox;
            geometry::translate(cellBox, mtv.x() / 2.0, mtv.y() / 2.0, newBox);
            cellBox = newBox;

            auto cell1Location = mDesign.placement().cellLocation(*cellIt);
            mDesign.placement().placeCell(*cellIt, util::Location(cell1Location.toPoint().x() + mtv.x()/2.0, cell1Location.toPoint().y() + mtv.y()/2.0));

            auto cell2Location = mDesign.placement().cellLocation(node.second);
            mDesign.placement().placeCell(node.second, util::Location(cell2Location.toPoint().x() - mtv.x()/2.0, cell2Location.toPoint().y() - mtv.y()/2.0));

            double mtvNorm = std::sqrt(mtv.x() * mtv.x() + mtv.y() + mtv.y());
            mtv.x(mtv.x() / mtvNorm);
            mtv.y(mtv.y() / mtvNorm);

            double ua = (mCellVelocities[*cellIt].x() * mtv.x()) + (mCellVelocities[*cellIt].y() * mtv.y());
            double ub = (mCellVelocities[node.second].x() * mtv.x()) + (mCellVelocities[node.second].y() * mtv.y());

            double massRelation = (mCellMasses[*cellIt] * mCellMasses[node.second]) / (mCellMasses[*cellIt] + mCellMasses[node.second]);
            double velocityRelation = ub - ua;
            double impulse = massRelation * velocityRelation;

            mCellImpulses[*cellIt].x(mtv.x() * impulse / 2.0);
            mCellImpulses[*cellIt].x(mtv.y() * impulse / 2.0);
            mCellImpulses[node.second].x(-mtv.x() * impulse / 2.0);
            mCellImpulses[node.second].x(-mtv.y() * impulse / 2.0);

        }
    }
}

void PhysicalEngineLegalization::findMinimumTranslationVector(geometry::Box &box1, geometry::Box &box2, geometry::Point & mtv)
{
    double leftMovement = box2.min_corner().x() - box1.max_corner().x();
    double rightMovement = box2.max_corner().x() - box1.min_corner().x();
    double downMovement = box2.min_corner().y() - box1.max_corner().y();
    double upMovement = box2.max_corner().y() - box1.min_corner().y();

    double xMovement = (std::abs(leftMovement) > std::abs(rightMovement)) ? leftMovement : rightMovement;
    double yMovement = (std::abs(downMovement) > std::abs(upMovement)) ? downMovement : upMovement;

    if (std::abs(xMovement) > std::abs(yMovement)) {
        mtv.x(xMovement);
    } else {
        mtv.y(yMovement);
    }
}
}
}

