#include <iostream>
#include "./wrapper/wrapper.h"
#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <string>



void runMultirowAbacusForOneCircuit(std::string tech_lef, std::string cell_lef, std::string input_def, unsigned int cpu, std::string placement_constraints, std::string output_def){
    Wrapper iccad(tech_lef, cell_lef, input_def);

    unsigned movableCells = 0;
    unsigned fixedCells = 0;

    ophidian::util::micrometer_t siteWidth(200);
    ophidian::util::micrometer_t rowHeight(2000);
    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::util::Location> initialLocations(iccad.mNetlist.makeProperty<ophidian::util::Location>(ophidian::circuit::Cell()));
    ophidian::entity_system::Property<ophidian::circuit::Cell, bool> initialFixed(iccad.mNetlist.makeProperty<bool>(ophidian::circuit::Cell()));
    for (auto cellIt = iccad.mNetlist.begin(ophidian::circuit::Cell()); cellIt != iccad.mNetlist.end(ophidian::circuit::Cell()); ++cellIt) {
        if (iccad.mPlacement.isFixed(*cellIt)) {
            fixedCells++;
        } else {
            movableCells++;
        }

        initialLocations[*cellIt] = iccad.mPlacement.cellLocation(*cellIt);
        initialFixed[*cellIt] = iccad.mPlacement.isFixed(*cellIt);

        auto cellLocation = iccad.mPlacement.cellLocation(*cellIt);
        auto alignedX = std::floor(units::unit_cast<double>(cellLocation.x() / siteWidth)) * siteWidth;
        auto alignedY = std::floor(units::unit_cast<double>(cellLocation.y() / rowHeight)) * rowHeight;
        iccad.mPlacement.placeCell(*cellIt, ophidian::util::Location(alignedX, alignedY));
    }

    ophidian::legalization::MultirowAbacus multirowAbacus(iccad.mNetlist, iccad.mFloorplan, iccad.mPlacement, iccad.mPlacementMapping);
    multirowAbacus.legalizePlacement();


    for (auto cellIt = iccad.mNetlist.begin(ophidian::circuit::Cell()); cellIt != iccad.mNetlist.end(ophidian::circuit::Cell()); ++cellIt) {
        iccad.mPlacement.fixLocation(*cellIt, initialFixed[*cellIt]);
    }


    iccad.writeDefFile(output_def);
}

int main(int argc, char** argv){
    if (argc != 13) {
        std::cout << "Error, usage: ./cada001 -tech_lef tech.lef -cell_lef cell.lef -input_def placed.def -cpu 4 -placement_constraints placement.constraints -output_def lg.def" << std::endl;
        return -1;
    }
    if(std::string(argv[1]) == "-tech_lef" && std::string(argv[3]) == "-cell_lef" && std::string(argv[5]) == "-input_def" && std::string(argv[7]) == "-cpu" && std::string(argv[9]) == "-placement_constraints" && std::string(argv[11]) == "-output_def")
        runMultirowAbacusForOneCircuit(argv[2], argv[4], argv[6], std::stoi(argv[8]), argv[10], argv[12]);
    else{
        std::cout << "Error, usage: ./cada001 -tech_lef tech.lef -cell_lef cell.lef -input_def placed.def -cpu 4 -placement_constraints placement.constraints -output_def lg.def" << std::endl;
        return -1;
    }
}
