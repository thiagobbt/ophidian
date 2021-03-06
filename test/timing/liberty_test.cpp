/*
 * Copyright 2016 Ophidian
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
 */

#include "../catch.hpp"

#include "../timing/liberty.h"

#include <boost/units/systems/si/prefixes.hpp>

TEST_CASE("liberty/pin capacitance", "[timing][liberty]") {
	ophidian::standard_cell::standard_cells std_cells;
	ophidian::timing::library_timing_arcs tarcs { &std_cells };
	ophidian::timing::library lib { &tarcs, &std_cells };
    ophidian::timing::liberty::read("input_files/superblue16_Early.lib", lib);
	REQUIRE(lib.pin_capacitance(std_cells.pin_create(std_cells.cell_create("NOR2_X4"), "b")) == boost::units::quantity<boost::units::si::capacitance>(3 * boost::units::si::femto * boost::units::si::farads));
	REQUIRE(lib.pin_capacitance(std_cells.pin_create(std_cells.cell_create("NOR2_X4"), "o")) == boost::units::quantity<boost::units::si::capacitance>(0.0 * boost::units::si::femto * boost::units::si::farads));
}

TEST_CASE("liberty/timing arcs", "[timing][liberty]") {
	ophidian::standard_cell::standard_cells std_cells;

	auto NOR2_X4 = std_cells.cell_create("NOR2_X4");
	auto NOR2_X4a = std_cells.pin_create(NOR2_X4, "a");
	auto NOR2_X4b = std_cells.pin_create(NOR2_X4, "b");
	auto NOR2_X4o = std_cells.pin_create(NOR2_X4, "o");

	ophidian::timing::library_timing_arcs tarcs { &std_cells };
	ophidian::timing::library lib { &tarcs, &std_cells };
    ophidian::timing::liberty::read("input_files/superblue16_Early.lib", lib);

	auto o_timing_arcs = lib.pin_timing_arcs(NOR2_X4o);

	REQUIRE(o_timing_arcs.size() == 2);

	bool froma { false }, fromb { false };

	for (auto arc : o_timing_arcs) {
		if (lib.timing_arc_from(arc) == NOR2_X4a)
			froma = true;
		else if (lib.timing_arc_from(arc) == NOR2_X4b)
			fromb = true;
		REQUIRE(lib.timing_arc_to(arc) == NOR2_X4o);
	}
	REQUIRE((froma && fromb));

	auto a_timing_arcs = lib.pin_timing_arcs(NOR2_X4a);
	REQUIRE(a_timing_arcs.size() == 1);
	REQUIRE(a_timing_arcs.front() == lib.timing_arc(NOR2_X4a, NOR2_X4o));
	REQUIRE(lib.timing_arc_from(a_timing_arcs.front()) == NOR2_X4a);
	REQUIRE(lib.timing_arc_to(a_timing_arcs.front()) == NOR2_X4o);
	auto b_timing_arcs = lib.pin_timing_arcs(NOR2_X4b);
	REQUIRE(b_timing_arcs.size() == 1);
	REQUIRE(b_timing_arcs.front() == lib.timing_arc(NOR2_X4b, NOR2_X4o));
	REQUIRE(lib.timing_arc_from(b_timing_arcs.front()) == NOR2_X4b);
	REQUIRE(lib.timing_arc_to(b_timing_arcs.front()) == NOR2_X4o);

}

TEST_CASE("liberty/lut", "[timing][liberty]") {
	ophidian::standard_cell::standard_cells std_cells;
	auto NOR2_X4 = std_cells.cell_create("NOR2_X4");
	auto NOR2_X4a = std_cells.pin_create(NOR2_X4, "a");
	auto NOR2_X4b = std_cells.pin_create(NOR2_X4, "b");
	auto NOR2_X4o = std_cells.pin_create(NOR2_X4, "o");
	ophidian::timing::library_timing_arcs tarcs { &std_cells };
	ophidian::timing::library lib { &tarcs, &std_cells };
    ophidian::timing::liberty::read("input_files/superblue16_Early.lib", lib);
	auto arc = lib.timing_arc(NOR2_X4a, NOR2_X4o);
	const ophidian::timing::library::LUT & fall_delay = lib.timing_arc_fall_delay(arc);
	REQUIRE( (fall_delay.row_count() == 7) );
	REQUIRE(fall_delay.column_count() == 8);
	REQUIRE(fall_delay.at(3, 4) == boost::units::quantity<boost::units::si::time>(52.84 * boost::units::si::pico * boost::units::si::seconds));
	REQUIRE(fall_delay.row_value(2) == boost::units::quantity<boost::units::si::capacitance>(4.00*boost::units::si::femto*boost::units::si::farads));
}


TEST_CASE("liberty/timing sense", "[timing][liberty]") {
	ophidian::standard_cell::standard_cells std_cells;
	auto NOR2_X4 = std_cells.cell_create("NOR2_X4");
	auto NOR2_X4a = std_cells.pin_create(NOR2_X4, "a");
	auto NOR2_X4b = std_cells.pin_create(NOR2_X4, "b");
	auto NOR2_X4o = std_cells.pin_create(NOR2_X4, "o");
	ophidian::timing::library_timing_arcs tarcs { &std_cells };
	ophidian::timing::library lib { &tarcs, &std_cells };
    ophidian::timing::liberty::read("input_files/superblue16_Early.lib", lib);
	auto arc = lib.timing_arc(NOR2_X4a, NOR2_X4o);
	REQUIRE( lib.timing_arc_timing_sense(arc) == ophidian::timing::unateness::NEGATIVE_UNATE );
}

TEST_CASE("liberty/pin direction", "[timing][liberty]") {
	ophidian::standard_cell::standard_cells std_cells;
	auto NOR2_X4 = std_cells.cell_create("NOR2_X4");
	auto NOR2_X4a = std_cells.pin_create(NOR2_X4, "a");
	auto NOR2_X4b = std_cells.pin_create(NOR2_X4, "b");
	auto NOR2_X4o = std_cells.pin_create(NOR2_X4, "o");
	ophidian::timing::library_timing_arcs tarcs { &std_cells };
	ophidian::timing::library lib { &tarcs, &std_cells };
    ophidian::timing::liberty::read("input_files/superblue16_Early.lib", lib);
	REQUIRE( std_cells.pin_direction(NOR2_X4a) == ophidian::standard_cell::pin_directions::INPUT );
	REQUIRE( std_cells.pin_direction(NOR2_X4b) == ophidian::standard_cell::pin_directions::INPUT );
	REQUIRE( std_cells.pin_direction(NOR2_X4o) == ophidian::standard_cell::pin_directions::OUTPUT );

}

TEST_CASE("liberty/flop", "[timing][liberty]") {
    ophidian::standard_cell::standard_cells std_cells;
    auto DFF_X80 = std_cells.cell_create("DFF_X80");
    auto DFF_X80ck = std_cells.pin_create(DFF_X80, "ck");
    auto DFF_X80d = std_cells.pin_create(DFF_X80, "d");
    auto DFF_X80q = std_cells.pin_create(DFF_X80, "q");
    ophidian::timing::library_timing_arcs tarcs { &std_cells };
    ophidian::timing::library lib { &tarcs, &std_cells };
    ophidian::timing::liberty::read("input_files/superblue16_Early.lib", lib);
    REQUIRE( std_cells.pin_direction(DFF_X80ck) == ophidian::standard_cell::pin_directions::INPUT );
    REQUIRE( std_cells.pin_direction(DFF_X80d) == ophidian::standard_cell::pin_directions::INPUT );
    REQUIRE( std_cells.pin_direction(DFF_X80q) == ophidian::standard_cell::pin_directions::OUTPUT );
    REQUIRE( std_cells.cell_sequential(DFF_X80) );
    REQUIRE( !std_cells.pin_clock_input(DFF_X80d) );
    REQUIRE( !std_cells.pin_clock_input(DFF_X80q) );
    REQUIRE( std_cells.pin_clock_input(DFF_X80ck) );
}


TEST_CASE("liberty/setup", "[timing][liberty]") {
    ophidian::standard_cell::standard_cells std_cells;
    auto DFF_X80 = std_cells.cell_create("DFF_X80");
    auto DFF_X80ck = std_cells.pin_create(DFF_X80, "ck");
    auto DFF_X80d = std_cells.pin_create(DFF_X80, "d");
    auto DFF_X80q = std_cells.pin_create(DFF_X80, "q");
    ophidian::timing::library_timing_arcs tarcs { &std_cells };
    ophidian::timing::library lib { &tarcs, &std_cells };
    ophidian::timing::liberty::read("input_files/simple_Late.lib", lib);



    using namespace boost::units;
    using namespace boost::units::si;



    REQUIRE( lib.setup_rise(lib.timing_arc(DFF_X80ck, DFF_X80d)).compute(0.0*seconds, 0.0*seconds) == quantity<si::time>(1.5*pico*seconds) );
    REQUIRE( lib.setup_fall(lib.timing_arc(DFF_X80ck, DFF_X80d)).compute(0.0*seconds, 0.0*seconds) == quantity<si::time>(2.5*pico*seconds) );
//    REQUIRE_THROWS( lib.setup_fall(lib.timing_arc(DFF_X80ck, DFF_X80q)) );


}


