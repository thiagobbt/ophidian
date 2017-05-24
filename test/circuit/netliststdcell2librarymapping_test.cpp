#include "netliststdcell2librarymapping_test.h"
#include <catch.hpp>

#include <ophidian/circuit/NetlistStdCell2LibraryMapping.h>
#include <ophidian/circuit/Verilog2Netlist.h>
#include <ophidian/circuit/Netlist.h>
#include <ophidian/standard_cell/StandardCells.h>
#include <ophidian/parser/Lef.h>
#include <sstream>

using namespace ophidian;

const std::string simpleVerilog = "module simple (\n"
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

void lef2StdCell(const parser::Lef & lef, standard_cell::StandardCells & stdCells){
    for(auto & macro : lef.macros())
    {
        stdCells.add(standard_cell::Cell(), macro.name);
        for(auto pin : macro.pins)
            stdCells.add(standard_cell::Pin(), macro.name+":"+pin.name, standard_cell::PinDirection(pin.direction));
    }
}

class NetlistStdCell2LibraryMappingFixture{
public:
    NetlistStdCell2LibraryMappingFixture():libraryMapping(netlist){
        parser::VerilogParser verilogParser;
        std::stringstream input(simpleVerilog);
        std::unique_ptr<parser::Verilog> verilog(verilogParser.readStream(input));
        circuit::verilog2Netlist(*verilog, netlist);

        parser::LefParser lefParser;
        std::unique_ptr<parser::Lef> lef(lefParser.readFile("./input_files/simple.lef"));
        lef2StdCell(*lef, stdCells);

        circuit::netlistStdCell2LibraryMapping(netlist, stdCells, *verilog, libraryMapping);
    }

    circuit::Netlist netlist;
    standard_cell::StandardCells stdCells;
    circuit::LibraryMapping libraryMapping;
};

TEST_CASE_METHOD(NetlistStdCell2LibraryMappingFixture, "NetlistStdCell2LibraryMapping: Test library cell mapping with simple circuit.", "[standard_cell][parser][lef][VerilogParser][simple][library_mapping][cell]")
{
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "u1")) == stdCells.find(standard_cell::Cell(), "NAND2_X1"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "u2")) == stdCells.find(standard_cell::Cell(), "NOR2_X1"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "u3")) == stdCells.find(standard_cell::Cell(), "INV_X1"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "u4")) == stdCells.find(standard_cell::Cell(), "INV_X1"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "f1")) == stdCells.find(standard_cell::Cell(), "DFF_X80"));
    REQUIRE(libraryMapping.cellStdCell(netlist.find(circuit::Cell(), "lcb1")) == stdCells.find(standard_cell::Cell(), "INV_Z80"));
}

TEST_CASE_METHOD(NetlistStdCell2LibraryMappingFixture, "NetlistStdCell2LibraryMapping: Test library pin mapping with simple circuit.", "[standard_cell][parser][lef][VerilogParser][simple][library_mapping][cell]")
{
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "u1:a")) == stdCells.find(standard_cell::Pin(), "NAND2_X1:a"));
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "u1:b")) == stdCells.find(standard_cell::Pin(), "NAND2_X1:b"));
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "u1:o")) == stdCells.find(standard_cell::Pin(), "NAND2_X1:o"));
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "lcb1:a")) == stdCells.find(standard_cell::Pin(), "INV_Z80:a"));
    REQUIRE(libraryMapping.pinStdCell(netlist.find(circuit::Pin(), "lcb1:o")) == stdCells.find(standard_cell::Pin(), "INV_Z80:o"));
}
