#include <SFML/Graphics.hpp>

#include <stdlib.h>
#include <ophidian/design/Design.h>
#include <ophidian/design/DesignBuilder.h>
#include <ophidian/legalization/KDtreeLegalization.h>




#include <thread>
#include <chrono>

///home/sheiny/workspace/benchmarks/ICCAD2017/fft_2_md2/tech.lef /home/sheiny/workspace/benchmarks/ICCAD2017/fft_2_md2/cells_modified.lef /home/sheiny/workspace/benchmarks/ICCAD2017/fft_2_md2/placed.def
///home/sheiny/workspace/benchmarks/ICCAD2017/des_perf_b_md2/tech.lef /home/sheiny/workspace/benchmarks/ICCAD2017/des_perf_b_md2/cells_modified.lef /home/sheiny/Desktop/des_perf_b_md2_lg.def
int main(int argc, char** argv){
    std::string lefFile = argv[1];
    std::string techFile = argv[2];
    std::string defFIle = argv[3];

    ophidian::designBuilder::ICCAD2017ContestDesignBuilder designBuilder(lefFile, techFile, defFIle);
    designBuilder.build();
    ophidian::design::Design & design = designBuilder.design();

    auto chipUpperCorner = design.floorplan().chipUpperRightCorner().toPoint();

    ophidian::geometry::Point windowSize(1024, 768);
    ophidian::geometry::Point scaleFactor(windowSize.x() / chipUpperCorner.x(), windowSize.y() / chipUpperCorner.y());

    sf::RenderWindow window(sf::VideoMode(windowSize.x(), windowSize.y()), defFIle);


    ophidian::legalization::KDtreeLegalization kdtree(design);
    kdtree.build();

    std::vector<sf::RectangleShape> boxes;
    boxes.reserve(design.netlist().size(ophidian::circuit::Cell()));

    unsigned int i = 4;
    kdtree.splitTree(i);

    //add partitions rectangles to draw
    for(auto subTree : kdtree.subTrees()){
        auto partitionBox = subTree.second;
        sf::RectangleShape shape(sf::Vector2f(partitionBox.max_corner().x() - partitionBox.min_corner().x(),
                                              partitionBox.max_corner().y() - partitionBox.min_corner().y()));
        shape.setFillColor(sf::Color(std::rand() % 255, std::rand() % 255, std::rand() % 255));

        auto height = partitionBox.max_corner().y() - partitionBox.min_corner().y();

        auto partitionLocation = partitionBox.min_corner();
        shape.setPosition(partitionLocation.x()*scaleFactor.x(), windowSize.y()-(partitionLocation.y() + height)*scaleFactor.y());

        shape.setScale(scaleFactor.x(), scaleFactor.y());

        boxes.push_back(shape);
    }

    //add cells box to draw
    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); cellIt++) {
        auto cellGeometry = design.placementMapping().geometry(*cellIt);
        auto cellBox = cellGeometry[0];
        sf::RectangleShape shape(sf::Vector2f(cellBox.max_corner().x() - cellBox.min_corner().x(),
                                              cellBox.max_corner().y() - cellBox.min_corner().y()));
        shape.setFillColor(sf::Color::Cyan);

        auto height = cellBox.max_corner().y() - cellBox.min_corner().y();

        auto cellLocation = design.placement().cellLocation(*cellIt).toPoint();
        shape.setPosition(cellLocation.x()*scaleFactor.x(), windowSize.y()-(cellLocation.y() + height)*scaleFactor.y());

        shape.setScale(scaleFactor.x(), scaleFactor.y());

        boxes.push_back(shape);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();

        for (auto box : boxes) {
            window.draw(box);
        }

        window.display();
    }

    return 0;
}
