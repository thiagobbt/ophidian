#include "CellAlignment.h"

namespace ophidian {
namespace legalization {
CellAlignment::CellAlignment(design::Design & design)
    : mDesign(design) {

}

void CellAlignment::alignCellsToSitesAndRows()
{
    auto site = *mDesign.floorplan().sitesRange().begin();
    auto siteWidth = mDesign.floorplan().siteUpperRightCorner(site).x();
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(site).y();

    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
        auto cell = *cellIt;

        auto cellLocation = mDesign.placement().cellLocation(cell);
        auto newX = std::round(units::unit_cast<double>(cellLocation.x() / siteWidth)) * siteWidth;
        auto newY = std::round(units::unit_cast<double>(cellLocation.y() / rowHeight)) * rowHeight;

        util::Location newLocation(newX, newY);
        mDesign.placement().placeCell(cell, newLocation);
    }
}

}
}
