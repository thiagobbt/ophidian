#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

#include <ophidian/placement/Def2Placement.h>
#include <ophidian/placement/Library.h>
#include <ophidian/parser/VerilogParser.h>
#include <ophidian/circuit/Verilog2Netlist.h>
#include <ophidian/circuit/LibraryMapping.h>
#include <ophidian/design/DesignBuilder.h>
#include <ophidian/design/Design.h>
#include <ophidian/geometry/Models.h>

#define USEDEBUG
#ifdef USEDEBUG
#define debug(x) std::cout << x
#else
#define debug(x)
#endif

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace op = ophidian;
namespace opd = ophidian::design;
namespace opc = ophidian::circuit;
namespace opp = ophidian::placement;

typedef op::geometry::Point point;
typedef bg::model::box<point> box;
typedef std::pair<box, opc::Cell> rnode;

bgi::rtree< rnode, bgi::rstar<16> > rtree;

bool isNodeTouchingBox(rnode const& node, box const& b) {
    auto node_box = std::get<0>(node);
    if (node_box.max_corner().x() <= b.min_corner().x() ||
        b.max_corner().x() <= node_box.min_corner().x() ||
        node_box.max_corner().y() <= b.min_corner().y() ||
        b.max_corner().y() <= node_box.min_corner().y()) {
        return false;
    }
    return true;
}

bool moveCell(opd::Design& design, opc::Cell cell, point targetLocation) {
    bgi::rtree< rnode, bgi::rstar<16> > tmpTree(rtree);

    auto std_cell = design.libraryMapping().cellStdCell(cell);
    auto cell_geometry = design.library().geometry(std_cell)[0];
    auto cell_location_point = design.placement().cellLocation(cell).toPoint();

    box targetBox(
        targetLocation,
        point(targetLocation.x() + cell_geometry.max_corner().x(), targetLocation.y() + cell_geometry.max_corner().y())
    );

    box originalBox(
        point(cell_location_point.x(), cell_location_point.y()),
        point(cell_location_point.x() + cell_geometry.max_corner().x(), cell_location_point.y() + cell_geometry.max_corner().y())
    );

    // Get r-tree node for the cell
    std::vector<rnode> possibleCells;
    tmpTree.query(bgi::covered_by(originalBox), std::back_inserter(possibleCells));
    debug("Found " << possibleCells.size() << " possible nodes for cell" << std::endl);

    auto cell_node_it = std::find_if(possibleCells.begin(), possibleCells.end(), [&cell](const rnode &n) {
        return std::get<1>(n) == cell;
    });

    if (cell_node_it == possibleCells.end()) {
        debug("Error: Original cell was not found." << std::endl);
        return false;
    }

    rnode cell_node = *cell_node_it;

    units::length::micrometer_t chipMinX = design.floorplan().chipOrigin().x();
    units::length::micrometer_t chipMaxX = design.floorplan().chipUpperRightCorner().x();

    tmpTree.remove(cell_node);

    // TODO: Check if cell fits in target rows

    // Fill a vector with all nodes that overlap the target box
    std::vector<rnode> overlaps_target;
    tmpTree.query(bgi::intersects(targetBox) && bgi::satisfies([&](rnode const& n) { return isNodeTouchingBox(n, targetBox); }), std::back_inserter(overlaps_target));
    debug("Overlaps: " << overlaps_target.size() << std::endl);

    // Vector to store all the movements to be realized or discarded if the placement fails
    std::vector<rnode> movements;
    rnode new_cell_node = std::make_pair(targetBox, cell);
    movements.push_back(new_cell_node);

    tmpTree.insert(new_cell_node);
    debug("Placed cell (" << design.netlist().name(cell) << ") at box {(" << targetBox.min_corner().x() << ", " << targetBox.min_corner().y() << "), ("
        << targetBox.max_corner().x() << ", " << targetBox.max_corner().y() << ")}" << std::endl);

    // Vector with pairs of overlapping cells
    std::vector<std::pair<rnode, rnode>> overlaps;

    for (auto & n : overlaps_target) {
        overlaps.push_back(make_pair(new_cell_node, n));
    }

    bool touchCircuitEdge = false;
    bool touchFixedCell = false;

    while (overlaps.size() > 0) {
        auto overlap = overlaps.back();
        overlaps.pop_back();

        auto node_i = std::get<0>(overlap);
        auto node_j = std::get<1>(overlap);

        // if (design.placement().isFixed(std::get<1>(node_j))) {
        //     debug("Cell j can't be moved, swapping i and j" << std::endl);

        //     auto tmp = node_i;
        //     node_i = node_j;
        //     node_j = tmp;

        //     touchFixedCell = true;
        //     invertCondition = true;
        // } else {
        //     touchFixedCell = false;
        // }

        auto i_name = design.netlist().name(std::get<1>(node_i));
        auto j_name = design.netlist().name(std::get<1>(node_j));

        debug(std::endl);
        debug("Treating {" << i_name << ", " << j_name << "} overlap" << std::endl);

        rnode new_j;

        auto min_corner_i = std::get<0>(node_i).min_corner();
        auto max_corner_i = std::get<0>(node_i).max_corner();
        auto width_i = max_corner_i.x() - min_corner_i.x();

        auto min_corner_j = std::get<0>(node_j).min_corner();
        auto max_corner_j = std::get<0>(node_j).max_corner();
        auto width_j = max_corner_j.x() - min_corner_j.x();

        tmpTree.remove(node_j);
        box target_j;

        // bool moveLeft = (min_corner_j.x() + width_j/2) <= (min_corner_i.x() + width_i/2);
        bool moveLeft = (min_corner_j.x() + width_j/2) <= (cell_location_point.x() + cell_geometry.max_corner().x());

        if (moveLeft) {
            debug("left " << width_j << std::endl);
            if ((min_corner_i.x() - width_j) < chipMinX.to<double>()) {
                // target_j = box(
                //     point(chipMinX.to<double>(), min_corner_j.y()),
                //     point(chipMinX.to<double>() + width_j, max_corner_j.y())
                // );
                // touchCircuitEdge = true;
                debug("Placement failed" << std::endl);
                return false;
            } else {
                target_j = box(
                    point(min_corner_i.x() - width_j, min_corner_j.y()),
                    point(min_corner_i.x(), max_corner_j.y())
                );
                touchCircuitEdge = false;
            }
        } else {
            debug("right " << width_j<< std::endl);
            if ((max_corner_i.x() + width_j) > chipMaxX.to<double>()) {
                // target_j = box(
                //     point(chipMaxX.to<double>() - width_j, min_corner_j.y()),
                //     point(chipMaxX.to<double>(), max_corner_j.y())
                // );
                // touchCircuitEdge = true;
                debug("Placement failed" << std::endl);
                return false;
            } else {
                target_j = box(
                    point(max_corner_i.x(), min_corner_j.y()),
                    point(max_corner_i.x() + width_j, max_corner_j.y())
                );
            }
            touchCircuitEdge = false;
        }

        new_j = std::make_pair(target_j, std::get<1>(node_j));
        movements.push_back(new_j);

        std::vector<rnode> overlaps_new_j;
        tmpTree.query(bgi::intersects(target_j) && bgi::satisfies([&](rnode const& n) { return isNodeTouchingBox(n, target_j); }), std::back_inserter(overlaps_new_j));
        debug("Found " << overlaps_new_j.size() << " new overlaps: " << std::endl);
        for (auto & n : overlaps_new_j) {
            if (std::get<1>(new_j) == std::get<1>(n)) continue;
            overlaps.push_back(make_pair(new_j, n));
            debug("    " << "{" << j_name << ", " << design.netlist().name(std::get<1>(n)) << "}" << std::endl);
        }

        tmpTree.insert(new_j);
    }

    debug("Finished treating overlaps" << std::endl);

    for (auto & n : movements) {
        double cellX = std::get<0>(n).min_corner().x();
        double cellY = std::get<0>(n).min_corner().y();
        ophidian::util::Location cellLocation(cellX, cellY);
        design.placement().placeCell(std::get<1>(n), cellLocation);
    }

    // debug(std::endl << "Tree after placement: " << std::endl);
    // std::vector<rnode> a;
    // tmpTree.query(bgi::nearest(point(0,0), 5), std::back_inserter(a));
    // for (auto & n : a) {
    //     std::string name = netlist.name(std::get<1>(n));
    //     auto min_corner = std::get<0>(n).min_corner();
    //     auto max_corner = std::get<0>(n).max_corner();
    //     debug(name << ": {(" << min_corner.x() << ", " << min_corner.y() << "), ("
    //         << max_corner.x() << ", " << max_corner.y() << ")}" << std::endl);
    // }
    // debug(std::endl << std::endl);

    rtree = tmpTree;

    return true;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Error: Number of parameters is invalid." << std::endl;
        return 1;
    }

    std::string def_path = argv[1];
    std::string verilog_path = argv[2];
    std::string lef_path = argv[3];

    op::designBuilder::ICCAD2015ContestDesignBuilder builder(lef_path, def_path, verilog_path);
    builder.build();

    opd::Design& design = builder.design();
    opc::Netlist& netlist = design.netlist();
    opp::Placement& placement = design.placement();
    opc::LibraryMapping& libraryMapping = design.libraryMapping();
    opp::Library& library = design.library();

    std::cout << "Building the r-tree" << std::endl;
    for (auto cell_it = netlist.begin(opc::Cell()); cell_it != netlist.end(opc::Cell()); cell_it++) {
        auto cell = *cell_it;
        auto std_cell = libraryMapping.cellStdCell(cell);

        auto cell_geometry = library.geometry(std_cell)[0];

        // std::cout << cell_geometry.min_corner().x() << " ~ " << cell_geometry.max_corner().x() << std::endl;

        auto cell_location_point = placement.cellLocation(cell).toPoint();

        box b(cell_location_point, point(cell_location_point.x() + cell_geometry.max_corner().x(), cell_location_point.y() + cell_geometry.max_corner().y()));
        rtree.insert(std::make_pair(b, cell));
    }
    std::cout << "Done building r-tree" << std::endl;

    design.setInputDefPath(def_path);

    // auto a_area = box(point(0, 76600), point(6000, 89800));
    // std::vector<rnode> a_area_nodes;
    // rtree.query(bgi::intersects(a_area) && bgi::satisfies([&](rnode const& n) { return isNodeTouchingBox(n, a_area); }), std::back_inserter(a_area_nodes));
    // std::cout << "Found " << a_area_nodes.size() << " nodes" << std::endl;

    // // int max_row = 89600;
    // // int row_num = 0;
    // int counter = 0;
    // int fail_counter = 0;
    // std::string dummy;
    // for (auto & node : a_area_nodes) {
    //     // int target_row = max_row - (row_num * 200);
    //     // row_num = (row_num + 1) % 71;

    //     // moveCell(design, std::get<1>(node), point(3000, target_row));
    //     if (!moveCell(design, std::get<1>(node), point(36460, 0))) fail_counter++;
    //     counter++;
    //     if (fail_counter > 0) break;
    //     if (counter % 10 == 0) {
    //         std::cout << "Moved " << counter << " cells (" << fail_counter << " failed)" << std::endl;
    //         std::string name = "test" + std::to_string(counter) + ".def";
    //         design.writeDefFile(name);
    //         // std::cin >> dummy;
    //     }
    //     if (counter == 20) break;
    // }

    // design.writeDefFile("test2.def");
    // std::cout << "Saved placement to " << "test2.def" << std::endl;

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
        } else if (cellName == "x") {
            std::cout << "Exiting" << std::endl;
            break;
        } else {
            std::cout << "Moving cell" << std::endl;
            int x, y;
            std::cin >> x >> y;

            auto cell = netlist.find(opc::Cell(), cellName);
            moveCell(design, cell, point(x, y));
            auto std_cell = design.libraryMapping().cellStdCell(cell);
            auto cell_geometry = design.library().geometry(std_cell)[0];
            auto cell_location_point = design.placement().cellLocation(cell).toPoint();
            std::cout << "Moved cell " << cellName << " to (" << cell_location_point.x() << ", " << cell_location_point.y() << ")" << std::endl;
        }
    }
}
