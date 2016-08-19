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

#ifndef OPHIDIAN_SUBROWS_H
#define OPHIDIAN_SUBROWS_H

#include <boost/geometry/index/rtree.hpp>
#include "../floorplan/floorplan.h"
#include "../placement/placement.h"
#include "../entity_system/entity_system.h"

namespace ophidian {
namespace legalization {
using point = geometry::point<double>;
using box = geometry::box<point>;
using polygon = geometry::polygon<point>;
using multi_polygon = geometry::multi_polygon<polygon>;
using rtree_node = std::pair<box, entity_system::entity>;
using rtree = boost::geometry::index::rtree<rtree_node, boost::geometry::index::rstar<16>>;
class subrows {

    const floorplan::floorplan & m_floorplan;

    entity_system::entity_system & m_system;

    entity_system::vector_property<double> m_begin;
    entity_system::vector_property<double> m_end;
    entity_system::vector_property<double> m_y;
    entity_system::vector_property<entity_system::entity> m_row;

    rtree subrows_rtree;
public:

    subrows(const floorplan::floorplan & floorplan, entity_system::entity_system & system)
        : m_floorplan(floorplan), m_system(system){
        m_system.register_property(&m_begin);
        m_system.register_property(&m_end);
        m_system.register_property(&m_y);
        m_system.register_property(&m_row);
    }

    std::size_t count() {
        return m_system.size();
    }

    void create_subrows(const std::vector<multi_polygon> & obstacles);

    void add_obstacle(const box obstacle);

    double begin(entity_system::entity subrow) const {
        return m_begin[m_system.lookup(subrow)];
    }
    void begin(entity_system::entity subrow, double begin);

    double end(entity_system::entity subrow) const {
        return m_end[m_system.lookup(subrow)];
    }
    void end(entity_system::entity subrow, double end);

    double y(entity_system::entity subrow) const {
        return m_y[m_system.lookup(subrow)];
    }

    double y(entity_system::entity subrow, double y);

    entity_system::entity row(entity_system::entity subrow) const {
        return m_row[m_system.lookup(subrow)];
    }
    void row(entity_system::entity subrow, entity_system::entity row);

    entity_system::entity find_subrow(point coordinate) const;

    std::vector<entity_system::entity> find_closest_subrows(point coordinate, unsigned number_of_rows) const;

};

class subrow_not_found : public std::exception {
public:
    const char * what() const throw() {
        return "No subrow found in the given coordinate";
    }
};
}
}


#endif //OPHIDIAN_SUBROWS_H
