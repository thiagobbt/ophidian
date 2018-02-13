#include "Abacus.h"

namespace ophidian
{
namespace legalization
{

using namespace ophidian::placement;

Abacus::Abacus(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping)
    : netlist_(netlist), floorplan_(floorplan), placement_(placement), placementMapping_(placementMapping),
    subrows_(netlist, floorplan, placement, placementMapping),
    cellName_(abacusCells_),
    abacusCell2NetlistCell_(abacusCells_), cellInitialLocations_(abacusCells_), cellLegalLocations_(abacusCells_), cellWidths_(abacusCells_), cellHeights_(abacusCells_), cellWeights_(abacusCells_),
    subrowCells_(subrows_.makeProperty<std::vector<AbacusCell> >(Subrow())),
    cells2Subrow_(abacusCells_),
    abacusPlaceRow_(subrows_, cellInitialLocations_, cellLegalLocations_, cellWidths_, cellWeights_){

}

void Abacus::legalizePlacement()
{
    geometry::Box chipArea(floorplan_.chipOrigin().toPoint(), floorplan_.chipUpperRightCorner().toPoint());
    util::MultiBox legalizationArea({chipArea});
    subrows_.createSubrows(legalizationArea);

    std::vector<std::pair<AbacusCell, util::Location> > sortedCells;
    sortedCells.reserve(netlist_.size(circuit::Cell()));
    for (auto cell_it = netlist_.begin(circuit::Cell()); cell_it != netlist_.end(circuit::Cell()); ++cell_it)
    {
        if (!placement_.isFixed(*cell_it))
        {
            auto abacus_cell = abacusCells_.add();
            abacusCell2NetlistCell_[abacus_cell] = *cell_it;
            cellInitialLocations_[abacus_cell] = placement_.cellLocation(*cell_it);
            auto cellGeometry = placementMapping_.geometry(*cell_it);
            cellWidths_[abacus_cell] = ophidian::util::micrometer_t(cellGeometry[0].max_corner().x() - cellGeometry[0].min_corner().x());
            cellHeights_[abacus_cell] = ophidian::util::micrometer_t(cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y());
            cellWeights_[abacus_cell] = std::max((int)netlist_.pins(*cell_it).size(), 1);
            cellName_[abacus_cell] = netlist_.name(*cell_it);
            sortedCells.push_back(std::make_pair(abacus_cell, placement_.cellLocation(*cell_it)));
        }
    }
    std::sort(sortedCells.begin(), sortedCells.end(), CellPairComparator());

    legalize(sortedCells);
}

bool Abacus::legalize(const std::vector<std::pair<AbacusCell, util::Location> > &sortedCells)
{
    util::micrometer_t siteWidth = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).x();
    util::micrometer_t chipTop = floorplan_.chipUpperRightCorner().y();

    unsigned cellIndex = 0;

    for (auto cellPair : sortedCells)
    {
        cellIndex++;

        auto abacusCell = cellPair.first;
        double bestCost = std::numeric_limits<double>::max();
        Subrow bestSubrow;
        unsigned rowsToSearch = 5;

        auto cellName = cellName_[abacusCell];

        while (bestCost == std::numeric_limits<double>::max())
        {
//            if (rowsToSearch == subrows_.rowCount()) {
//                std::cout << "stop " << std::endl;
//            }

//            if (cellIndex == 239) {
//                std::cout << "stop " << std::endl;
//            }

            std::vector<Subrow> closeSubrows;
            closeSubrows.reserve(rowsToSearch);
            subrows_.findClosestSubrows(rowsToSearch, cellInitialLocations_[abacusCell], closeSubrows);
            for (auto subrow : closeSubrows)
            {
                auto capacity = subrows_.capacity(subrow);
                auto cellWidth = cellWidths_[abacusCell];
                auto cellHeight = cellHeights_[abacusCell];
                auto origin = subrows_.origin(subrow).y();
                if ((subrows_.capacity(subrow) >= cellWidths_[abacusCell]) && ((subrows_.origin(subrow).y() + cellHeights_[abacusCell]) <= chipTop))
                {
                    subrowCells_[subrow].push_back(abacusCell);
                    abacusPlaceRow_(subrow, subrowCells_[subrow], siteWidth);
                    auto cellLegalLocation = cellLegalLocations_[abacusCell];
                    auto cellInitialLocation = cellInitialLocations_[abacusCell];
                    double cost = std::abs(units::unit_cast<double>(cellLegalLocations_[abacusCell].x()) - units::unit_cast<double>(cellInitialLocations_[abacusCell].x())) +
                                  std::abs(units::unit_cast<double>(cellLegalLocations_[abacusCell].y()) - units::unit_cast<double>(cellInitialLocations_[abacusCell].y()));
                    subrowCells_[subrow].pop_back();
                    if (cost < bestCost)
                    {
                        bestCost = cost;
                        bestSubrow = subrow;
                    }
                }
            }
            if (rowsToSearch == subrows_.rowCount()) {
                break;
            } else {
                rowsToSearch *= 2;
                if (rowsToSearch > subrows_.rowCount()) {
                    rowsToSearch = subrows_.rowCount();
                }
            }
        }

        if (bestCost != std::numeric_limits<double>::max()) {
            auto subrowOrigin = subrows_.origin(bestSubrow);
            subrowCells_[bestSubrow].push_back(abacusCell);
            subrows_.capacity(bestSubrow, subrows_.capacity(bestSubrow) - cellWidths_[abacusCell]);

            cells2Subrow_[abacusCell] = bestSubrow;
        }else
            return false;
    }

//    double averageCellsPerSubrow = 0;
//    std::size_t maxCellsPerSubrow = 0;
    for (auto subrow : subrows_.range(Subrow()))
    {
        auto subrowOrigin = subrows_.origin(subrow);
//        if (units::unit_cast<double>(subrowOrigin.y()) == 176000 && units::unit_cast<double>(subrowOrigin.x()) == 92400) {
//            std::cout << "stop " << std::endl;
//        }
        abacusPlaceRow_(subrow, subrowCells_[subrow], siteWidth);
//        averageCellsPerSubrow += subrowCells_[subrow].size();
//        maxCellsPerSubrow = std::max(maxCellsPerSubrow, subrowCells_[subrow].size());
    }
//    averageCellsPerSubrow /= subrows_.rowCount();
//    std::cout << "average cells per subrow " << averageCellsPerSubrow << std::endl;
//    std::cout << "max cells per subrow " << maxCellsPerSubrow << std::endl;

    for (auto cellPair : sortedCells)
    {
        if (cells2Subrow_[cellPair.first] != Subrow()) {
            auto netlistCell = abacusCell2NetlistCell_[cellPair.first];
            auto cellLocation = cellLegalLocations_[cellPair.first];
            placement_.placeCell(netlistCell, cellLocation);
        }
    }
    return true;
}

} // namespace legalization
} // namespace ophidian
