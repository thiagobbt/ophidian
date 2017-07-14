#include "MultirowAbacusFixture.h"

MultiRowAbacusFixture::MultiRowAbacusFixture(const ophidian::circuit::Netlist & netlist, const ophidian::floorplan::Floorplan & floorplan, ophidian::placement::Placement & placement, const ophidian::placement::PlacementMapping & placementMapping)
    : ophidian::legalization::MultirowAbacus(netlist, floorplan, placement, placementMapping) {

}

void MultiRowAbacusFixture::writeCsvWithCellsPerSubrow(std::string fileName)
{
    std::ofstream csvFile;
    csvFile.open (fileName);

    csvFile << "number_of_cells" << std::endl;
    for (auto subrow : subrows_.range(ophidian::legalization::Subrow())) {
        auto numberOfCells = subrowCells_[subrow].size();
        csvFile << numberOfCells << std::endl;
    }

    csvFile.close();
}
