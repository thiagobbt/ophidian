#include "verilog2netlist_test.h"
#include <catch.hpp>

#include <ophidian/circuit/Verilog2Netlist.h>
#include <ophidian/circuit/Netlist.h>
#include <ophidian/circuit/LibraryMapping.h>
#include <ophidian/standard_cell/StandardCells.h>
#include <sstream>

using namespace ophidian;

class Verilog2NetlistFixture{
public:
    Verilog2NetlistFixture():libraryMapping(netlist){
        parser::VerilogParser parser;
        std::stringstream input(simpleInput);
        verilog.reset(parser.readStream(input));
        circuit::verilog2Netlist(*verilog, netlist, libraryMapping, standardCells);
    }

    const std::string simpleInput = "module simple (\n"
                                    "inp1,\n"
                                    "inp2,\n"
                                    "iccad_clk,\n"
                                    "out\n"
                                    ");\n"
                                    "// Start PIs\n"
                                    "input inp1;\n"
                                    "input inp2;\n"
                                    "input iccad_clk;\n"
                                    "// Start POs\n"
                                    "output out;\n"
                                    "// Start wires\n"
                                    "wire n1;\n"
                                    "wire n2;\n"
                                    "wire n3;\n"
                                    "wire n4;\n"
                                    "wire inp1;\n"
                                    "wire inp2;\n"
                                    "wire iccad_clk;\n"
                                    "wire out;\n"
                                    "wire lcb1_fo;\n"
                                    "// Start cells\n"
                                    "NAND2_X1 u1 ( .a(inp1), .b(inp2), .o(n1) );\n"
                                    "NOR2_X1 u2 ( .a(n1), .b(n3), .o(n2) );\n"
                                    "DFF_X80 f1 ( .d(n2), .ck(lcb1_fo), .q(n3) );\n"
                                    "INV_X1 u3 ( .a(n3), .o(n4) );\n"
                                    "INV_X1 u4 ( .a(n4), .o(out) );\n"
                                    "INV_Z80 lcb1 ( .a(iccad_clk), .o(lcb1_fo) );\n"
                                    "endmodule\n";
    std::unique_ptr<parser::Verilog> verilog;
    circuit::Netlist netlist;
    circuit::LibraryMapping libraryMapping;
    standard_cell::StandardCells standardCells;
};

TEST_CASE_METHOD(Verilog2NetlistFixture, "Verilog2Netlist: The Verilog object and Netlist module must have same amount of elements.", "[circuit][Netlist][Verilog]")
{
    const parser::Verilog::Module & simple = verilog->modules().front();
    REQUIRE(simple.nets().size() == netlist.size(circuit::Net()));
    REQUIRE(simple.ports().size() == (netlist.size(circuit::Input()) + netlist.size(circuit::Output())));
    REQUIRE(simple.instances().size() == netlist.size(circuit::Cell()));

    std::size_t module_size_pins = 0;
    for(auto instance : simple.instances())
        module_size_pins += instance.portMapping().size();
    module_size_pins += simple.ports().size();
    REQUIRE(module_size_pins == netlist.size(circuit::Pin()));
}

TEST_CASE_METHOD(Verilog2NetlistFixture, "Verilog2Netlist: Fetching some entities by their names and check their names.", "[circuit][Netlist][Verilog]")
{
    const parser::Verilog::Module & simple = verilog->modules().front();
    REQUIRE(netlist.name(netlist.find(circuit::Pin(), "inp1")) == "inp1");
    REQUIRE(netlist.name(netlist.find(circuit::Pin(), "u3:a")) == "u3:a");
    REQUIRE(netlist.name(netlist.find(circuit::Cell(), "u1")) == "u1");
    REQUIRE(netlist.name(netlist.find(circuit::Net(), "iccad_clk")) == "iccad_clk");
}

TEST_CASE_METHOD(Verilog2NetlistFixture, "Verilog2Netlist: Test library cell mapping with simple circuit.", "[standard_cell][parser][lef][VerilogParser][simple][library_mapping][cell]")
{
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "u1")) == standardCells.find(standard_cell::Cell(), "NAND2_X1"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "u2")) == standardCells.find(standard_cell::Cell(), "NOR2_X1"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "u3")) == standardCells.find(standard_cell::Cell(), "INV_X1"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "u4")) == standardCells.find(standard_cell::Cell(), "INV_X1"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "f1")) == standardCells.find(standard_cell::Cell(), "DFF_X80"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "lcb1")) == standardCells.find(standard_cell::Cell(), "INV_Z80"));
}

TEST_CASE_METHOD(Verilog2NetlistFixture, "Verilog2Netlist: Test library pin mapping with simple circuit.", "[standard_cell][parser][lef][VerilogParser][simple][library_mapping][cell]")
{
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "u1:a")) == standardCells.find(standard_cell::Pin(), "NAND2_X1:a"));
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "u1:b")) == standardCells.find(standard_cell::Pin(), "NAND2_X1:b"));
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "u1:o")) == standardCells.find(standard_cell::Pin(), "NAND2_X1:o"));
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "lcb1:a")) == standardCells.find(standard_cell::Pin(), "INV_Z80:a"));
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "lcb1:o")) == standardCells.find(standard_cell::Pin(), "INV_Z80:o"));
}

TEST_CASE_METHOD(Verilog2NetlistFixture, "Verilog2Netlist: Test of association between netlist::Pin and netlist::Cell.", "[circuit][Netlist][Verilog][EntitySystem]")
{
    auto cellU1 = netlist.find(circuit::Cell(), "u1");
    auto netInp1 = netlist.find(circuit::Net(), "inp1");
    REQUIRE(netlist.pins(cellU1).size() == 3);
    REQUIRE(netlist.pins(netInp1).size() == 2);
}

TEST_CASE_METHOD(Verilog2NetlistFixture, "Verilog2Netlist: Test of association between standard_cell::Pin and standard_cell::Cell.", "[circuit][standard_cell][Verilog][EntitySystem]")
{
    auto cellNAND2X1 = standardCells.find(standard_cell::Cell(), "NAND2_X1");
    auto pincellNAND2X1a = standardCells.find(standard_cell::Pin(), "NAND2_X1:a");
    REQUIRE(standardCells.owner(pincellNAND2X1a) == cellNAND2X1);
    REQUIRE(standardCells.pins(cellNAND2X1).size() == 3);
}
