#include "MultirowAbacus.h"

namespace ophidian
{
namespace legalization
{
MultirowAbacus::MultirowAbacus(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping)
    : Abacus(netlist, floorplan, placement, placementMapping){

}

void MultirowAbacus::legalizeSubrows(std::vector<circuit::Cell> & cellsForOneHeight, unsigned rowsPerCell, unsigned subRowIndex, util::MultiBox legalizationArea) {
    if (rowsPerCell == 1) {
        subrows_.createSubrows(legalizationArea);
    } else {
        subrows_.createSubrows(legalizationArea, rowsPerCell, subRowIndex);
    }

    std::vector<std::pair<AbacusCell, util::Location> > sortedCells;
    sortedCells.reserve(cellsForOneHeight.size());
    for (auto cell : cellsForOneHeight)
    {
        auto abacus_cell = abacusCells_.add();
        abacusCell2NetlistCell_[abacus_cell] = cell;
        cellInitialLocations_[abacus_cell] = placement_.cellLocation(cell);
        auto cellGeometry = placementMapping_.geometry(cell);
        cellWidths_[abacus_cell] = ophidian::util::micrometer_t(cellGeometry[0].max_corner().x() - cellGeometry[0].min_corner().x());
        cellHeights_[abacus_cell] = ophidian::util::micrometer_t(cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y());
        cellWeights_[abacus_cell] = std::max((int)netlist_.pins(cell).size(), 1);
//        cellWeights_[abacus_cell] = netlist_.pins(cell).size();
        cellName_[abacus_cell] = netlist_.name(cell);
        sortedCells.push_back(std::make_pair(abacus_cell, placement_.cellLocation(cell)));
    }
    std::sort(sortedCells.begin(), sortedCells.end(), CellPairComparator());
    legalize(sortedCells);
    for (auto cell : cellsForOneHeight)
    {
        placement_.fixLocation(cell, true);
    }

    abacusCells_.clear();
}

void MultirowAbacus::legalizePlacement(std::vector<circuit::Cell> cells, util::MultiBox legalizationArea)
{
    ophidian::entity_system::Property<ophidian::circuit::Cell, bool> initialFixed(netlist_.makeProperty<bool>(ophidian::circuit::Cell()));
    auto rowHeight = floorplan_.rowUpperRightCorner(*floorplan_.rowsRange().begin()).y();

    std::vector<std::vector<circuit::Cell> > cellsByHeight;
    cellsByHeight.resize(10);
    unsigned maximumHeight = 1;
    for (auto cell : cells)
    {
        initialFixed[cell] = placement_.isFixed(cell);
        if (!placement_.isFixed(cell))
        {
            auto cellGeometry = placementMapping_.geometry(cell);
            auto cellHeight = ophidian::util::micrometer_t(cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y());
            unsigned heightInNumberOfRows = cellHeight / rowHeight;
            maximumHeight = std::max(maximumHeight, heightInNumberOfRows);
            cellsByHeight.at(heightInNumberOfRows - 1).push_back(cell);
        }
    }
    cellsByHeight.resize(maximumHeight);

    unsigned rowsPerCell = cellsByHeight.size();
    for (auto cellsByHeightIt = cellsByHeight.rbegin(); cellsByHeightIt != cellsByHeight.rend(); ++cellsByHeightIt)
    {
        std::vector<circuit::Cell> cellsForOneHeight = *cellsByHeightIt;
        auto cellGeometry = placementMapping_.geometry(*cellsForOneHeight.begin());
        auto cellHeight = ophidian::util::micrometer_t(cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y());
        auto siteHeight = floorplan_.siteUpperRightCorner(*floorplan_.sitesRange().begin()).y();
        if(std::fmod((cellHeight/siteHeight), 2.0))
        {
            //Odd-sized cells -> place in all rows
            legalizeSubrows(cellsForOneHeight, rowsPerCell, 0, legalizationArea);
        }
        else {
            //Even-sized cells -> place in specific rows
            //Separating cells by power rail alignment odd and even
            std::vector<circuit::Cell> cellsOdd;
            std::vector<circuit::Cell> cellsEven;
            cellsOdd.reserve(cellsForOneHeight.size());
            cellsEven.reserve(cellsForOneHeight.size());
            for(auto cell : cellsForOneHeight)
            {
                if(placementMapping_.alignment(cell) == placement::RowAlignment::EVEN)
                {
                    cellsEven.push_back(cell);
                }
                else {
                    cellsOdd.push_back(cell);
                }
            }
            legalizeSubrows(cellsEven, rowsPerCell, 0, legalizationArea);
            legalizeSubrows(cellsOdd, rowsPerCell, 1, legalizationArea);
        }
        rowsPerCell--;
    }

    for (auto cell : cells)
    {
        placement_.fixLocation(cell, initialFixed[cell]);
    }
}
} // namespace legalization
} // namespace ophidian
