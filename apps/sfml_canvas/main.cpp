#include <SFML/Graphics.hpp>

#include <ophidian/design/Design.h>
#include <ophidian/design/DesignBuilder.h>

int main(int argc, char** argv){
    std::string lefFile = argv[1];
    std::string defFile = argv[2];
    std::string verilogFile = argv[3];

    ophidian::designBuilder::ICCAD2015ContestDesignBuilder designBuilder(lefFile, defFile, verilogFile);
    designBuilder.build();
    ophidian::design::Design & design = designBuilder.design();

    auto chipUpperCorner = design.floorplan().chipUpperRightCorner().toPoint();

    ophidian::geometry::Point windowSize(1024, 768);
    ophidian::geometry::Point scaleFactor(windowSize.x() / chipUpperCorner.x(), windowSize.y() / chipUpperCorner.y());

    sf::RenderWindow window(sf::VideoMode(windowSize.x(), windowSize.y()), "SFML Canvas");

    std::vector<sf::RectangleShape> boxes;
    boxes.reserve(design.netlist().size(ophidian::circuit::Cell()));
    for (auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); cellIt++) {
        auto cellGeometry = design.placementMapping().geometry(*cellIt);
        auto cellBox = cellGeometry[0];
        sf::RectangleShape shape(sf::Vector2f(cellBox.max_corner().x() - cellBox.min_corner().x(),
                                              cellBox.max_corner().y() - cellBox.min_corner().y()));

        auto cellLocation = design.placement().cellLocation(*cellIt).toPoint();
        shape.setPosition(cellLocation.x(), cellLocation.y());

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
