#include <iostream>
#include "./wrapper/wrapper.h"
#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <string>

void runMultirowAbacusForOneCircuit(std::string circuitPath, std::string circuitName) {
    Wrapper iccad(circuitPath, circuitName);

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


    iccad.writeDefFile(circuitName + "_legalized.def");
}


int main(int argc, char** argv){
    if (argc < 3) {
        std::cout << "usage cada001 <circuit_pah> <circuit_name>" << std::endl;
        return -1;
    }
    runMultirowAbacusForOneCircuit(argv[1], argv[2]);
}
