#include "def2fence_test.h"

#include <catch.hpp>

#include "ophidian/parser/Def.h"
#include "ophidian/placement/Def2Fence.h"
#include "ophidian/placement/Def2Placement.h"
#include "ophidian/placement/Placement.h"
#include "ophidian/circuit/Netlist.h"
#include "ophidian/parser/Lef.h"
#include "ophidian/floorplan/Floorplan.h"
#include "ophidian/floorplan/LefDef2Floorplan.h"

using namespace ophidian;

TEST_CASE("Def2Fence: Test library cell geometries.", "[placement][Def2Fence]")
{
    parser::DefParser parser;
    std::unique_ptr<parser::Def> def = parser.readFile("./input_files/simple.def");
    circuit::Netlist netlist;
    placement::Placement placement(netlist);
    placement::Fences fences(netlist);

    floorplan::Floorplan floorplan;
    ophidian::parser::LefParser lefParser;
    auto mLef =  std::make_unique<ophidian::parser::Lef>();
    lefParser.readFile("./input_files/simple.lef", mLef);
    floorplan::lefDef2Floorplan(*mLef, *def, floorplan);

    placement::def2placement(*def, placement, netlist);
    placement::def2fence(*def, fences, netlist, placement, floorplan);

    REQUIRE(fences.size() == 2);

    auto region0 = fences.find("r0");
    auto region1 = fences.find("r1");

    SECTION("Def2Fence: Check fence areas")
    {
        auto r0area = fences.area(region0);
        auto r1area = fences.area(region1);

        REQUIRE(r0area == util::MultiBox({geometry::Box({0, 0}, {500400, 146000})}));
        REQUIRE(r1area == util::MultiBox({
            geometry::Box({305800, 92000}, {502400, 146000}),
            geometry::Box({305800, 214000}, {502400, 264000})
        }));
    }

    SECTION("Def2Fence: Check fence members")
    {
        std::vector<circuit::Cell> r0cells;
        r0cells.push_back(netlist.find(circuit::Cell(), "u1"));
        r0cells.push_back(netlist.find(circuit::Cell(), "u2"));
        r0cells.push_back(netlist.find(circuit::Cell(), "u3"));
        r0cells.push_back(netlist.find(circuit::Cell(), "u4"));

        REQUIRE(std::is_permutation(r0cells.begin(), r0cells.end(), fences.members(region0).begin()));

        std::vector<circuit::Cell> r1cells;
        r1cells.push_back(netlist.find(circuit::Cell(), "f1"));
        r1cells.push_back(netlist.find(circuit::Cell(), "lcb1"));
        REQUIRE(std::is_permutation(r1cells.begin(), r1cells.end(), fences.members(region1).begin()));
    }

    SECTION("Def2Fence: Check if components have a fence")
    {
        auto lcb1 = netlist.find(ophidian::circuit::Cell(), "lcb1");
        auto lcb1Fence = placement.cellFence(lcb1);

        REQUIRE(lcb1Fence == region1);
    }
}
