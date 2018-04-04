// #include "def2netlist_test.h"
#include <catch.hpp>

#include "ophidian/circuit/Def2Netlist.h"
#include "ophidian/circuit/Verilog2Netlist.h"
#include "ophidian/circuit/Netlist.h"
#include <sstream>

using namespace ophidian;

class Def2NetlistFixture
{
public:
	Def2NetlistFixture(){
		ophidian::parser::DefParser defParser;
    	def = defParser.readFile("input_files/simple/simple.def");

    	parser::VerilogParser verilogParser;
    	verilog = std::unique_ptr<parser::Verilog>(verilogParser.readFile("input_files/simple/simple.v"));

        circuit::def2Netlist(*def, netlistDef);
        circuit::verilog2Netlist(*verilog, netlistVerilog);
	}

	std::unique_ptr<parser::Def> def;
	std::unique_ptr<parser::Verilog> verilog;
	circuit::Netlist netlistDef;
	circuit::Netlist netlistVerilog;
};

TEST_CASE_METHOD(Def2NetlistFixture, "Def2Netlist: Netlists from verilog and def have the same number of elements", "[circuit][Netlist][Def]")
{
	REQUIRE(netlistDef.size(circuit::Net())    == netlistVerilog.size(circuit::Net()));
	REQUIRE(netlistDef.size(circuit::Input())  == netlistVerilog.size(circuit::Input()));
	REQUIRE(netlistDef.size(circuit::Output()) == netlistVerilog.size(circuit::Output()));
	REQUIRE(netlistDef.size(circuit::Cell())   == netlistVerilog.size(circuit::Cell()));
	REQUIRE(netlistDef.size(circuit::Pin())    == netlistVerilog.size(circuit::Pin()));

	for (auto itDef = netlistDef.begin(circuit::Net()),
		 itVerilog = netlistVerilog.begin(circuit::Net());
		 itDef < netlistDef.end(circuit::Net());
		 itDef++, itVerilog++)
	{
		REQUIRE(netlistDef.pins(*itDef).size() == netlistVerilog.pins(*itVerilog).size());
	}
}
