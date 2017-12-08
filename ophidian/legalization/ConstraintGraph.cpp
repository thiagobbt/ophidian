#include "ConstraintGraph.h"

namespace ophidian {
namespace legalization {

bool LeftComparator::operator()(util::Location &cell1Location, util::Location &cell2Location, geometry::Box &cell1Box, geometry::Box &cell2Box)
{
    if (cell1Location.x() > cell2Location.x()) {
        return false;
    }

    auto cell1Width = cell1Box.max_corner().x() - cell1Box.min_corner().x();
    auto cell1Height = cell1Box.max_corner().y() - cell1Box.min_corner().y();
    auto cell2Height = cell2Box.max_corner().y() - cell2Box.min_corner().y();

    double xDistance = cell2Location.toPoint().x() - (cell1Location.toPoint().x() + cell1Width);
    double yDistance = (cell1Location.toPoint().y() <= cell2Location.toPoint().y()) ? cell2Location.toPoint().y() - (cell1Location.toPoint().y() + cell1Height) : cell1Location.toPoint().y() - (cell2Location.toPoint().y() + cell2Height);

    return (xDistance > yDistance);
}

double LeftComparator::arcCost(geometry::Box &cell1Box)
{
    return cell1Box.max_corner().x() - cell1Box.min_corner().x();
}

bool BelowComparator::operator()(util::Location &cell1Location, util::Location &cell2Location, geometry::Box &cell1Box, geometry::Box &cell2Box)
{
    if (cell1Location.y() > cell2Location.y()) {
        return false;
    }

    auto cell1Width = cell1Box.max_corner().x() - cell1Box.min_corner().x();
    auto cell1Height = cell1Box.max_corner().y() - cell1Box.min_corner().y();
    auto cell2Width = cell2Box.max_corner().x() - cell2Box.min_corner().x();

    double xDistance = (cell1Location.toPoint().x() <= cell2Location.toPoint().x()) ? cell2Location.toPoint().x() - (cell1Location.toPoint().x() + cell1Width) : cell1Location.toPoint().x() - (cell2Location.toPoint().x() + cell2Width);
    double yDistance = cell2Location.toPoint().y() - (cell1Location.toPoint().y() + cell1Height);

    return (yDistance >= xDistance);
}

double BelowComparator::arcCost(geometry::Box &cell1Box)
{
    cell1Box.max_corner().y() - cell1Box.min_corner().y();
}

}
}
