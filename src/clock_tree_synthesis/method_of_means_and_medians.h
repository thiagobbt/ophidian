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


#ifndef METHOD_OF_MEANS_AND_MEDIANS_H
#define METHOD_OF_MEANS_AND_MEDIANS_H

#include "clock_tree_definition.h"
#include "clock_tree_algorithm.h"
#include "topology.h"
#include "embedding.h"
#include "../geometry/geometry.h"


namespace ophidian {
namespace clock_tree_synthesis {


class method_of_means_and_medians : public clock_tree_algorithm
{
public:
    using point = geometry::point<PointType>;

    class coordinate_comparator{
        public:
            coordinate_comparator(bool sort_by_x_coordinate) : m_sort_by_x_coordinate(sort_by_x_coordinate){}
            bool operator()(const point & position1, const point & position2) const {
                if(m_sort_by_x_coordinate)
                    return position1.x() < position2.x();
                else
                    return position1.y() < position2.y();
            }
        private:
            bool m_sort_by_x_coordinate;
   };

private:
    void build_topology_internal( topology::node parent_node, std::vector<point>::iterator positions_begin, std::vector<point>::iterator positions_end, topology &clock_topology, embedding &embedding_for_sinks, bool sort_by_x_coordinate);

public:
    method_of_means_and_medians();
    ~method_of_means_and_medians();

    void build_topology(const point & clock_source, const std::vector<point> & flip_flop_positions, clock_tree_synthesis::topology & clock_topology, embedding &embedding_for_sinks);
    void build(const topology & clock_topology, embedding & embedding);

};

}
}

#endif // METHOD_OF_MEANS_AND_MEDIANS_H
