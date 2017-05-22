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

#include "NetlistStdCell2LibraryMapping.h"

namespace ophidian {
namespace circuit {
void netlistStdCell2LibraryMapping(Netlist & netlist, standard_cell::StandardCells & stdCells, const parser::Verilog & verilog, LibraryMapping & libraryMapping){
    const parser::Verilog::Module & module = verilog.modules().front();
    for(auto instance : module.instances()){
        auto netlistCell = netlist.find(Cell(), instance.name());
        auto stdCell = stdCells.find(standard_cell::Cell(), instance.module()->name());
        libraryMapping.cellStdCell(netlistCell, stdCell);

        for(auto portMap : instance.portMapping()){
            auto netlistPin = netlist.find(Pin(), instance.name()+":"+portMap.first->name());
            auto stdPin = stdCells.find(standard_cell::Pin(), instance.module()->name()+":"+portMap.first->name());
            libraryMapping.pinStdCell(netlistPin, stdPin);
        }
    }
}
}
}
