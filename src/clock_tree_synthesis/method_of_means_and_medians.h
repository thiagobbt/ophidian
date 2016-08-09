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

#include "clock_topology.h"
#include "embedding.h"
#include "../geometry/geometry.h"

namespace ophidian {
namespace clock_tree_synthesis {

template <class PointType>
class method_of_means_and_medians
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
    void build_topology(typename clock_topology<PointType>::node parent_node, typename std::vector<point>::iterator positions_begin, typename std::vector<point>::iterator positions_end, clock_topology<PointType> &topology, embedding<PointType> & embedding, bool sort_by_x_coordinate)
    {
        point center_of_mass = calculate_center_of_mass(positions_begin, positions_end);
        auto center_of_mass_node = topology.node_create();
        embedding.node_position(center_of_mass_node, center_of_mass);
        topology.edge_create(parent_node, center_of_mass_node);

        auto size = std::distance(positions_begin, positions_end);
        if (size > 1) {
            std::sort(positions_begin, positions_end, coordinate_comparator(sort_by_x_coordinate));
            auto positions_middle = std::next(positions_begin, std::ceil(size / 2.0));
            build_topology(center_of_mass_node, positions_begin, positions_middle, topology, embedding, !sort_by_x_coordinate);
            build_topology(center_of_mass_node, positions_middle, positions_end, topology, embedding, !sort_by_x_coordinate);
        }
    }

    point calculate_center_of_mass(typename std::vector<point>::iterator positions_begin, typename std::vector<point>::iterator positions_end)
    {
        std::size_t size = std::distance(positions_begin, positions_end);
        point center_of_mass{0.0, 0.0};
        for (auto position = positions_begin; position != positions_end; ++position) {
            center_of_mass.x(center_of_mass.x() + position->x());
            center_of_mass.y(center_of_mass.y() + position->y());
        }
        center_of_mass.x(center_of_mass.x() / (double)size);
        center_of_mass.y(center_of_mass.y() / (double)size);
        return center_of_mass;
    }

public:
    method_of_means_and_medians()
    {

    }

    ~method_of_means_and_medians()
    {

    }

    void build_topology(const point clock_source, const std::vector<point> & flip_flop_positions, clock_tree_synthesis::clock_topology<PointType> &clock_topology, clock_tree_synthesis::embedding<PointType> &embedding)
    {
        std::vector<method_of_means_and_medians<PointType>::point> flip_flop_positions_copy = flip_flop_positions;
        auto source_node = clock_topology.node_create();
        embedding.node_position(source_node, clock_source);
        build_topology(source_node, flip_flop_positions_copy.begin(), flip_flop_positions_copy.end(), clock_topology, embedding, true);
    }

    void build_topology(const point clock_source, const std::vector<point> & flip_flop_positions, clock_tree_synthesis::clock_topology<PointType> & clock_topology)
    {
        clock_tree_synthesis::embedding<PointType> embedding(clock_topology.graph());
        std::vector<method_of_means_and_medians::point> flip_flop_positions_copy = flip_flop_positions;
        auto source_node = clock_topology.node_create();
        embedding.node_position(source_node, clock_source);
        build_topology(source_node, flip_flop_positions_copy.begin(), flip_flop_positions_copy.end(), clock_topology, embedding, true);
    }
};

}
}

#endif // METHOD_OF_MEANS_AND_MEDIANS_H
