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
//    iccad2017.kdtreeLegalization();
    iccad2017.legalize();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout<<"Seconds: "<<std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()<<std::endl;

    if(ophidian::legalization::legalizationCheck(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()))
        std::cout<<"Circuit is legalized."<<std::endl;
    else
        std::cout<<"Circuit is ilegal."<<std::endl;

    std::cout<<"Total Displacement: "<<quality.totalDisplacement()<<std::endl;
    std::cout<<"Average Displacement: "<<quality.avgDisplacement()<<std::endl;
    std::cout<<"Maximum Cell Movement(lines): "<<quality.maximumCellMovement()<<std::endl;

//    design.writeDefFile(output_def);
}

void runMultirowAbacusICCAD2017(std::string tech_lef, std::string cell_lef, std::string input_def, unsigned int cpu, std::string placement_constraints, std::string output_def){
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder(cell_lef, tech_lef, input_def);
    ICCAD2017DesignBuilder.build();
    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();


    ophidian::legalization::iccad2017Legalization iccad2017(design);

    ophidian::legalization::ICCAD2017SolutionQuality quality(design);

    auto t1 = std::chrono::high_resolution_clock::now();
//    iccad2017.kdtreeLegalization();
    iccad2017.legalize();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout<<"Seconds: "<<std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()<<std::endl;

    if(ophidian::legalization::legalizationCheck(design.floorplan(), design.placement(), design.placementMapping(), design.netlist(), design.fences()))
        std::cout<<"Circuit is legalized."<<std::endl;
    else
        std::cout<<"Circuit is ilegal."<<std::endl;

    std::cout<<"Total Displacement: "<<quality.totalDisplacement()<<std::endl;
    std::cout<<"Average Displacement: "<<quality.avgDisplacement()<<std::endl;
    std::cout<<"Maximum Cell MoverunMultirowAbacusForOneCircuitment(lines): "<<quality.maximumCellMovement()<<std::endl;

//    design.writeDefFile(output_def);
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

int main(int argc, char** argv){
    randomPlacementWrite(argv[2], argv[4], argv[6], argv[8]);
}

//ICCAD 2015
//int main(int argc, char** argv){
//    if (argc != 9)
//    {
//        std::cout << "Error, usage: ./cada001 -lef superblue.lef -def superblue.def -verilog superblue.v -output_def lg.def" << std::endl;
//        return -1;
//    }
//    std::vector<std::string> circuits({"superblue10", "superblue18", "superblue4", "superblue7", "superblue1", "superblue16", "superblue3", "superblue5"});
//    if(std::string(argv[1]) == "-lef" && std::string(argv[3]) == "-def" && std::string(argv[5]) == "-verilog" && std::string(argv[7]) == "-output_def")
//        for(auto circuit : circuits)
//            runMultirowAbacusICCAD2015(argv[2], argv[4], argv[6], argv[8]);
////        runMultirowAbacusICCAD2015(argv[2], argv[4], argv[6], argv[8]);
//    else {
//        std::cout << "Error, usage: ./cada001 -lef superblue.lef -def superblue.def -verilog superblue.v -output_def lg.def" << std::endl;
//        return -1;
//    }
//}


//ICCAD 2017
//int main(int argc, char** argv){
//    if (argc != 13)
//    {
//        std::cout << "Error, usage: ./cada001 -tech_lef tech.lef -cell_lef cell.lef -input_def placed.def -cpu 4 -placement_constraints placement.constraints -output_def lg.def" << std::endl;
//        return -1;
//    }
//    std::vector<std::string> circuits({"des_perf_1", "des_perf_b_md2", "fft_2_md2", "pci_bridge32_a_md2", "des_perf_a_md1", "edit_dist_1_md1", "fft_a_md2", "pci_bridge32_b_md1", "des_perf_a_md2", "edit_dist_a_md2", "fft_a_md3", "pci_bridge32_b_md2", "des_perf_b_md1", "edit_dist_a_md3", "pci_bridge32_a_md1", "pci_bridge32_b_md3"});
//    if(std::string(argv[1]) == "-tech_lef" && std::string(argv[3]) == "-cell_lef" && std::string(argv[5]) == "-input_def" && std::string(argv[7]) == "-cpu" && std::string(argv[9]) == "-placement_constraints" && std::string(argv[11]) == "-output_def")
//        for(auto circuit: circuits)
//        {
//            std::cout<<"Runing circuit: "<<circuit<<std::endl;
//            runMultirowAbacusICCAD2017(argv[2]+circuit+"/tech.lef", argv[4]+circuit+"/cells_modified.lef", argv[6]+circuit+"/placed.def", std::stoi(argv[8]), argv[10]+circuit+"/placement.constraints", argv[12]+circuit+".def");
//        }
//        runMultirowAbacusICCAD2017(argv[2], argv[4], argv[6], std::stoi(argv[8]), argv[10], argv[12]);
//    else {
//        std::cout << "Error, usage: ./cada001 -tech_lef tech.lef -cell_lef cell.lef -input_def placed.def -cpu 4 -placement_constraints placement.constraints -output_def lg.def" << std::endl;
//        return -1;
//    }
//}
