#include <iostream>
#include <limits>

#include <ophidian/design/DesignBuilder.h>
#include <ophidian/geometry/Models.h>
#include <ophidian/legalization/CellLegalizer.h>

#include <ophidian/legalization/iccad2017Legalization.h>

typedef ophidian::geometry::Point point;
typedef boost::geometry::model::box<point> Box;

// int main(int argc, char** argv) {
//     if (argc != 4) {
//         std::cerr << "Error: Number of parameters is invalid." << std::endl;
//         return 1;
//     }

//     std::string def_path = argv[1];
//     std::string verilog_path = argv[2];
//     std::string lef_path = argv[3];

//     ophidian::designBuilder::ICCAD2015ContestDesignBuilder builder(lef_path, def_path, verilog_path);
//     builder.build();

//     auto& design = builder.design();
//     design.setInputDefPath(def_path);

//     ophidian::legalization::CellLegalizer legalizer(design);

//     std::string cellName = "";
//     while (true) {
//         std::cout << "Waiting for command: ";
//         std::cin >> cellName;

//         if (cellName == "s") {
//             std::cout << "Saving placement" << std::endl;
//             std::string filename;
//             std::cin >> filename;
//             design.writeDefFile(filename);
//             std::cout << "Saved placement to " << filename << std::endl;
//         } else if (cellName == "x" || cellName == "q") {
//             std::cout << "Exiting" << std::endl;
//             break;
//         } else {
//             std::cout << "Moving cell" << std::endl;
//             int x, y;
//             std::cin >> x >> y;

//             auto cell = design.netlist().find(ophidian::circuit::Cell(), cellName);
//             bool status = legalizer.legalizeCell(cell, ophidian::geometry::Point(x, y), Box(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint()));

//             if (status) {
//                 std::cout << "Movement succeeded" << std::endl;
//             } else {
//                 std::cout << "Movement failed" << std::endl;
//             }
//         }

//         std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

//         std::cout << std::endl;
//     }
// }

// des_perf_b_md1

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }

    std::string circuitName = std::string(argv[1]);
    ophidian::designBuilder::ICCAD2017ContestDesignBuilder ICCAD2017DesignBuilder("./input_files/benchmarks2017/" + circuitName + "/cells_modified.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/tech.lef",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placed.def",
                                                                                  "./input_files/benchmarks2017/" + circuitName + "/placement.constraints");
    ICCAD2017DesignBuilder.build();

    ophidian::design::Design & design = ICCAD2017DesignBuilder.design();
    design.setInputDefPath("./input_files/benchmarks2017/" + circuitName + "/placed.def");

    ophidian::legalization::iccad2017Legalization iccad2017(design);
    std::cout << "Legalizing" << std::endl;
    iccad2017.legalize();
    std::cout << "Legalized" << std::endl;

    std::cout << "Writing DEF file" << std::endl;
    design.writeDefFile(circuitName + "_after_legalization.def");
    std::cout << "DEF written" << std::endl;

    ophidian::legalization::CellLegalizer legalizer(design);

    std::vector<ophidian::circuit::Cell> legalizedCells = {};

    std::string cellName = "";
    while (true) {
        std::cout << "Waiting for command: ";
        std::cin >> cellName;
        std::cout << std::endl;

        if (cellName == "s") {
            std::cout << "Saving placement" << std::endl;
            std::string filename;
            std::cin >> filename;
            design.writeDefFile(filename);
            std::cout << "Saved placement to " << filename << std::endl;
        } else if (cellName == "x") {
            std::cout << "Exiting" << std::endl;
            break;
        } else {
            std::cout << "Moving cell" << std::endl;
            int x, y;
            std::cin >> x >> y;

            auto cell = design.netlist().find(ophidian::circuit::Cell(), cellName);
            bool successful = legalizer.legalizeCell(cell, point(x, y), legalizedCells, Box(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint()));

            if (successful) {
                std::cout << "Moved cell" << std::endl;
                legalizedCells.push_back(cell);
            }
        }
    }
}
