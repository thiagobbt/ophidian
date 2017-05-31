#include "MultirowAbacus.h"

namespace ophidian {
namespace legalization {
MultirowAbacus::MultirowAbacus(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping)
    : Abacus(netlist, floorplan, placement, placementMapping){

}

void MultirowAbacus::legalizePlacement()
{
    auto rowHeight = floorplan_.rowUpperRightCorner(*floorplan_.rowsRange().begin()).y();

    std::vector<std::vector<circuit::Cell>> cellsByHeight;
    cellsByHeight.resize(1000);
    unsigned maximumHeight = 1;
    unsigned cellId = 0;
    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        if (!placement_.isFixed(*cellIt)) {
            cellId++;
            auto cellGeometry = placementMapping_.geometry(*cellIt);
            auto cellHeight = ophidian::util::micrometer_t(cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y());
            unsigned heightInNumberOfRows = cellHeight / rowHeight;
            maximumHeight = std::max(maximumHeight, heightInNumberOfRows);
            cellsByHeight.at(heightInNumberOfRows - 1).push_back(*cellIt);
        }
    }
    cellsByHeight.resize(maximumHeight);

    unsigned rowsPerCell = cellsByHeight.size();
    unsigned index = 0;
    for (auto cellsByHeightIt = cellsByHeight.rbegin(); cellsByHeightIt != cellsByHeight.rend(); ++cellsByHeightIt) {
        if (index == 55) {
            std::cout << "stop" << std::endl;
        }
        std::cout << "cells by height " << index++ << std::endl;

        subrows_.createSubrows(rowsPerCell);
        rowsPerCell--;

        auto cellsForOneHeight = *cellsByHeightIt;
        std::vector<std::pair<AbacusCell, util::Location>> sortedCells;
        sortedCells.reserve(cellsForOneHeight.size());
        for (auto cell : cellsForOneHeight) {
            auto abacus_cell = abacusCells_.add();
            abacusCell2NetlistCell_[abacus_cell] = cell;
            cellInitialLocations_[abacus_cell] = placement_.cellLocation(cell);
            auto cellGeometry = placementMapping_.geometry(cell);
            cellWidths_[abacus_cell] = ophidian::util::micrometer_t(cellGeometry[0].max_corner().x() - cellGeometry[0].min_corner().x());
            cellHeights_[abacus_cell] = ophidian::util::micrometer_t(cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y());
            cellWeights_[abacus_cell] = netlist_.pins(cell).size();
            sortedCells.push_back(std::make_pair(abacus_cell, placement_.cellLocation(cell)));
        }
        std::sort(sortedCells.begin(), sortedCells.end(), CellPairComparator());
        legalize(sortedCells);
        for (auto cell : cellsForOneHeight) {
            placement_.fixLocation(cell, true);
        }

        abacusCells_.clear();
    }
}
}
}
