#include <iostream>
#include "./wrapper/wrapper.h"
#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/legalization/LegalizationCheck.h>
#include <ophidian/legalization/iccad2017Legalization.h>
#include <ophidian/design/Design.h>
#include <ophidian/design/DesignBuilder.h>
#include <ophidian/legalization/iccad2017SolutionQuality.h>
#include <string>

#include <chrono>

void runMultirowAbacusICCAD2015(std::string lef, std::string def, std::string verilog, std::string output_def){
    ophidian::designBuilder::ICCAD2015ContestDesignBuilder ICCAD2015DesignBuilder(lef, def, verilog);
    ICCAD2015DesignBuilder.build();
    ophidian::design::Design & design = ICCAD2015DesignBuilder.design();

    ophidian::legalization::iccad2017Legalization iccad2017(design);

    ophidian::legalization::ICCAD2017SolutionQuality quality(design);


    auto t1 = std::chrono::high_resolution_clock::now();
//    iccad2017.ancientLegalization();
    iccad2017.legalize();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout<<"Seconds: "<<std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()<<std::endl;

    if(ophidian::legalization::legalizationCheck(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()))
        std::cout<<"Circuit is legalized."<<std::endl;
    else
        std::cout<<"Circuit is ilegal."<<std::endl;

    std::cout<<"Total Displacement: "<<quality.totalDisplacement()<<std::endl;
    std::cout<<"Maximum Cell Movement(lines): "<<quality.maximumCellMovement()<<std::endl;

    design.writeDefFile(output_def);
}

void runMultirowAbacusForOneCircuit(std::string tech_lef, std::string cell_lef, std::string input_def, unsigned int cpu, std::string placement_constraints, std::string output_def){
//    Wrapper iccad(tech_lef, cell_lef, input_def);

//    unsigned movableCells = 0;
//    unsigned fixedCells = 0;

//    ophidian::util::micrometer_t siteWidth(200);
//    ophidian::util::micrometer_t rowHeight(2000);
//    ophidian::entity_system::Property<ophidian::circuit::Cell, ophidian::util::Location> initialLocations(iccad.mNetlist.makeProperty<ophidian::util::Location>(ophidian::circuit::Cell()));
//    for (auto cellIt = iccad.mNetlist.begin(ophidian::circuit::Cell()); cellIt != iccad.mNetlist.end(ophidian::circuit::Cell()); ++cellIt)
//    {
//        if (iccad.mPlacement.isFixed(*cellIt))
//        {
//            fixedCells++;
//        }
//        else {
//            movableCells++;
//        }

//        initialLocations[*cellIt] = iccad.mPlacement.cellLocation(*cellIt);

//        auto cellLocation = iccad.mPlacement.cellLocation(*cellIt);
//        auto alignedX = std::floor(units::unit_cast<double>(cellLocation.x() / siteWidth)) * siteWidth;
//        auto alignedY = std::floor(units::unit_cast<double>(cellLocation.y() / rowHeight)) * rowHeight;
//        iccad.mPlacement.placeCell(*cellIt, ophidian::util::Location(alignedX, alignedY));
//    }

    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder(cell_lef, tech_lef, input_def);
    ICCAD2017DesignBuilder.build();
    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();


    ophidian::legalization::iccad2017Legalization iccad2017(design);

    ophidian::legalization::ICCAD2017SolutionQuality quality(design);

    auto t1 = std::chrono::high_resolution_clock::now();
//    iccad2017.ancientLegalization();
    iccad2017.legalize();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout<<"Seconds: "<<std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()<<std::endl;

    if(ophidian::legalization::legalizationCheck(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()))
        std::cout<<"Circuit is legalized."<<std::endl;
    else
        std::cout<<"Circuit is ilegal."<<std::endl;

    std::cout<<"Total Displacement: "<<quality.totalDisplacement()<<std::endl;
    std::cout<<"Maximum Cell Movement(lines): "<<quality.maximumCellMovement()<<std::endl;

    design.writeDefFile(output_def);
}

//ICCAD 2015
//int main(int argc, char** argv){
//    if (argc != 9)
//    {
//        std::cout << "Error, usage: ./cada001 -lef superblue.lef -def superblue.def -verilog superblue.v -output_def lg.def" << std::endl;
//        return -1;
//    }
//    if(std::string(argv[1]) == "-lef" && std::string(argv[3]) == "-def" && std::string(argv[5]) == "-verilog" && std::string(argv[7]) == "-output_def")
//        runMultirowAbacusICCAD2015(argv[2], argv[4], argv[6], argv[8]);
//    else {
//        std::cout << "Error, usage: ./cada001 -lef superblue.lef -def superblue.def -verilog superblue.v -output_def lg.def" << std::endl;
//        return -1;
//    }
//}


//ICCAD 2017
int main(int argc, char** argv){
    if (argc != 13)
    {
        std::cout << "Error, usage: ./cada001 -tech_lef tech.lef -cell_lef cell.lef -input_def placed.def -cpu 4 -placement_constraints placement.constraints -output_def lg.def" << std::endl;
        return -1;
    }
    if(std::string(argv[1]) == "-tech_lef" && std::string(argv[3]) == "-cell_lef" && std::string(argv[5]) == "-input_def" && std::string(argv[7]) == "-cpu" && std::string(argv[9]) == "-placement_constraints" && std::string(argv[11]) == "-output_def")
        runMultirowAbacusForOneCircuit(argv[2], argv[4], argv[6], std::stoi(argv[8]), argv[10], argv[12]);
    else {
        std::cout << "Error, usage: ./cada001 -tech_lef tech.lef -cell_lef cell.lef -input_def placed.def -cpu 4 -placement_constraints placement.constraints -output_def lg.def" << std::endl;
        return -1;
    }
}
