/*
 * Copyright 2017 Ophidian
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

#include "Def2Netlist.h"

namespace ophidian
{
namespace circuit
{

void def2Netlist(const parser::Def &def, Netlist &netlist){
	netlist.reserve(Net(), def.nets().size());

	for (auto defComponents : def.components()) {
		netlist.add(Cell(), defComponents.name);
	}

	for (auto defNet : def.nets()) {
		auto net = netlist.add(Net(), defNet.name);

		for (auto defPin : defNet.pins) {
			std::string pinName;

			if (defPin.instance != "PIN") pinName += defPin.instance + ":";
			pinName += defPin.name;

			auto pin = netlist.add(Pin(), pinName);

			if (defPin.instance != "PIN") {
				auto cell = netlist.add(Cell(), defPin.instance);
				netlist.add(cell, pin);
			}

			netlist.connect(net, pin);
		}
	}

	for (auto defPin : def.pins()) {
		auto pin = netlist.add(Pin(), defPin.name);

		if (defPin.direction == parser::Def::direction_t::INPUT)
			netlist.add(Input(), pin);
		else if (defPin.direction == parser::Def::direction_t::OUTPUT)
			netlist.add(Output(), pin);
	}
}
} // namespace circuit
} // namespace ophidian
