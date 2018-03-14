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
#include <random>

void runMultirowAbacusICCAD2015(std::string strategy, unsigned int i, std::string lef, std::string def, std::string verilog, std::string output_def){
    ophidian::designBuilder::ICCAD2015ContestDesignBuilder ICCAD2015DesignBuilder(lef, def, verilog);
    ICCAD2015DesignBuilder.build();
    ophidian::design::Design & design = ICCAD2015DesignBuilder.design();

    ophidian::legalization::iccad2017Legalization iccad2017(design);

    ophidian::legalization::ICCAD2017SolutionQuality quality(design);


    auto t1 = std::chrono::high_resolution_clock::now();
    if(strategy == "legalization")
        iccad2017.legalize();
    else if(strategy == "kdtreelegalization")
        iccad2017.kdtreeLegalization(i);
    else{
        std::cout<<"error wrong strategy"<<std::endl;
        return;
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout<<"Seconds: "<<std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()<<std::endl;

    if(ophidian::legalization::legalizationCheck(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()))
        std::cout<<"Circuit is legalized."<<std::endl;
    else
        std::cout<<"Circuit is ilegal."<<std::endl;

    std::cout<<"Total Displacement: "<<quality.totalDisplacement()<<std::endl;
    std::cout<<"Average Displacement: "<<quality.avgDisplacement()<<std::endl;
    std::cout<<"Maximum Cell Movement(lines): "<<quality.maximumCellMovement()<<std::endl;

    design.writeDefFile(output_def);
}

void runMultirowAbacusICCAD2017(std::string strategy, unsigned int i, std::string tech_lef, std::string cell_lef, std::string input_def, unsigned int cpu, std::string placement_constraints, std::string output_def){
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder(cell_lef, tech_lef, input_def);
    ICCAD2017DesignBuilder.build();
    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();


    ophidian::legalization::iccad2017Legalization iccad2017(design);

    ophidian::legalization::ICCAD2017SolutionQuality quality(design);

    auto t1 = std::chrono::high_resolution_clock::now();
    if(strategy == "legalization")
        iccad2017.legalize();
    else if(strategy == "kdtreelegalization")
        iccad2017.kdtreeLegalization(i);
    else{
        std::cout<<"error wrong strategy"<<std::endl;
        return;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout<<"Seconds: "<<std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()<<std::endl;

    if(ophidian::legalization::legalizationCheck(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()))
        std::cout<<"Circuit is legalized."<<std::endl;
    else
        std::cout<<"Circuit is ilegal."<<std::endl;

    std::cout<<"Total Displacement: "<<quality.totalDisplacement()<<std::endl;
    std::cout<<"Average Displacement: "<<quality.avgDisplacement()<<std::endl;
    std::cout<<"Maximum Cell Movement(lines): "<<quality.maximumCellMovement()<<std::endl;

    design.writeDefFile(output_def);
}

void randomPlacementWrite(std::string lef, std::string def, std::string verilog, std::string output_def){
    ophidian::designBuilder::ICCAD2015ContestDesignBuilder ICCAD2015DesignBuilder(lef, def, verilog);
    ICCAD2015DesignBuilder.build();
    ophidian::design::Design & design = ICCAD2015DesignBuilder.design();

    std::default_random_engine gen(42);
    std::uniform_int_distribution<> dis(-10000, 10000);

    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); cellIt++)
    {
        if (design.placement().isFixed(*cellIt) == false)
            design.placement().placeCell(*cellIt, ophidian::util::Location(design.placement().cellLocation(*cellIt).toPoint().x()+dis(gen),
                                                                           design.placement().cellLocation(*cellIt).toPoint().y()+dis(gen)));
    }
    design.writeDefFile(output_def);
}

//int main(int argc, char** argv){
//    randomPlacementWrite(argv[2], argv[4], argv[6], argv[8]);
//}

int main(int argc, char** argv){
    if (argc == 11){
        if(std::string(argv[3]) == "-lef" && std::string(argv[5]) == "-def" && std::string(argv[7]) == "-verilog" && std::string(argv[9]) == "-output_def")
            runMultirowAbacusICCAD2015(argv[1], std::stoi(argv[2]), argv[4], argv[6], argv[8], argv[10]);

    }else if(argc == 15){
        if(std::string(argv[3]) == "-tech_lef" && std::string(argv[5]) == "-cell_lef" && std::string(argv[7]) == "-input_def" && std::string(argv[9]) == "-cpu" && std::string(argv[11]) == "-placement_constraints" && std::string(argv[13]) == "-output_def")
            runMultirowAbacusICCAD2017(argv[1], std::stoi(argv[2]), argv[4], argv[6], argv[8], std::stoi(argv[10]), argv[12], argv[14]);
    }else{
        std::cout << "Error, usage: ./cada001 legalization/kdtreelegalization i -tech_lef tech.lef -cell_lef cell.lef -input_def placed.def -cpu 4 -placement_constraints placement.constraints -output_def lg.def" << std::endl;
        std::cout << "Error, usage: ./cada001 legalization/kdtreelegalization i -lef superblue.lef -def superblue.def -verilog superblue.v -output_def lg.def" << std::endl;
        return -1;
    }
}
