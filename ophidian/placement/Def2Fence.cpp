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

#include "Def2Fence.h"

#include <boost/algorithm/string/replace.hpp>
#include <numeric>
#include <regex>

namespace ophidian
{
namespace placement
{

void def2fence(const parser::Def &def, Fences &fences, circuit::Netlist &netlist, Placement &placement, floorplan::Floorplan &floorplan) {
    auto def_regions = def.regions();
    auto def_groups = def.groups();

    for (auto region : def_regions)
    {
        auto fence = fences.add(region.name);

        util::MultiBox mb(region.rectangles);
        ophidian::geometry::Box chipBox(floorplan.chipOrigin().toPoint(), floorplan.chipUpperRightCorner().toPoint());

        util::MultiBox intersection;
        for(auto box : mb)
        {
            ophidian::geometry::Box resp;
            boost::geometry::intersection(box, chipBox, resp);
            intersection.push_back(resp);
        }

        fences.area(fence, intersection);


        auto member_vector = def_groups[region.name];

        std::string members_regex_str =
            std::accumulate(member_vector.begin(), member_vector.end(), std::string(),
                            [](const std::string& a, const std::string& b) -> std::string {
                    return a + (a.length() > 0 ? "|" : "") + b;
                });

        boost::replace_all(members_regex_str, "*", ".*");
        boost::replace_all(members_regex_str, "/", "\\/");

        auto members_regex = std::regex(members_regex_str);

        for (auto component : def.components())
        {
            if (std::regex_match(component.name, members_regex))
            {
                auto netlist_cell = netlist.add(circuit::Cell(), component.name);

                fences.connect(fence, netlist_cell);
                placement.cellFence(netlist_cell, fence);
            }
        }
    }
}

} // namespace placement
} // namespace ophidian
