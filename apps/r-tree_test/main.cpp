#include <iostream>

#include <ophidian/design/DesignBuilder.h>
#include <ophidian/geometry/Models.h>
#include <ophidian/legalization/CellLegalizer.h>

typedef ophidian::geometry::Point point;
typedef boost::geometry::model::box<point> Box;

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Error: Number of parameters is invalid." << std::endl;
        return 1;
    }

    std::string def_path = argv[1];
    std::string verilog_path = argv[2];
    std::string lef_path = argv[3];

    ophidian::designBuilder::ICCAD2015ContestDesignBuilder builder(lef_path, def_path, verilog_path);
    builder.build();

    auto& design = builder.design();
    design.setInputDefPath(def_path);

    ophidian::legalization::CellLegalizer legalizer(design);

    std::string cellName = "";
    while (true) {
        std::cout << "Waiting for command: " << std::endl;
        std::cin >> cellName;
        std::cout << std::endl;

        if (cellName == "s") {
            std::cout << "Saving placement" << std::endl;
            std::string filename;
            std::cin >> filename;
            design.writeDefFile(filename);
            std::cout << "Saved placement to " << filename << std::endl;
        } else if (cellName == "x" || cellName == "q") {
            std::cout << "Exiting" << std::endl;
            break;
        } else {
            std::cout << "Moving cell" << std::endl;
            int x, y;
            std::cin >> x >> y;

            auto cell = design.netlist().find(ophidian::circuit::Cell(), cellName);
            bool status = legalizer.legalizeCell(cell, ophidian::geometry::Point(x, y), Box(design.floorplan().chipOrigin().toPoint(), design.floorplan().chipUpperRightCorner().toPoint()));

            if (status) {
                std::cout << "Movement succeeded" << std::endl;
            } else {
                std::cout << "Movement failed" << std::endl;
            }
        }
    }
}
