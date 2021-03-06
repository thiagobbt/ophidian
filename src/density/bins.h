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

#ifndef ophidian_BINS_H
#define ophidian_BINS_H

#include <vector_property.h>
#include <entity_system.h>
#include "../geometry/geometry.h"

namespace ophidian {
    namespace density {

        class bins {
            using point = geometry::point<double>;

            entity_system::entity_system & m_system;

            entity_system::vector_property<point> m_positions;
            entity_system::vector_property<point> m_dimensions;
            entity_system::vector_property<double> m_movable_utilization;
            entity_system::vector_property<double> m_fixed_utilization;
            entity_system::vector_property<double> m_free_space;

        public:

            bins(entity_system::entity_system & system) : m_system(system) {
                m_system.register_property(&m_positions);
                m_system.register_property(&m_dimensions);
                m_system.register_property(&m_movable_utilization);
                m_system.register_property(&m_fixed_utilization);
                m_system.register_property(&m_free_space);
            }

            virtual ~bins() { }

            point position(entity_system::entity bin) {
                return m_positions[m_system.lookup(bin)];
            }

            point dimension(entity_system::entity bin) {
                return m_dimensions[m_system.lookup(bin)];
            }

            double movable_utilization(entity_system::entity bin) {
                return m_movable_utilization[m_system.lookup(bin)];
            }

            double fixed_utilization(entity_system::entity bin) {
                return m_fixed_utilization[m_system.lookup(bin)];
            }

            double free_space(entity_system::entity bin) {
                return m_free_space[m_system.lookup(bin)];
            }

            std::pair< std::vector<point>::const_iterator, std::vector<point>::const_iterator > positions() const {
                return std::make_pair(m_positions.begin(), m_positions.end());
            }

            std::pair< std::vector<point>::const_iterator, std::vector<point>::const_iterator > dimensions() const {
                return std::make_pair(m_dimensions.begin(), m_dimensions.end());
            }

            std::pair< std::vector<double>::const_iterator, std::vector<double>::const_iterator > movable_utilizations() const {
                return std::make_pair(m_movable_utilization.begin(), m_movable_utilization.end());
            }

            std::pair< std::vector<double>::const_iterator, std::vector<double>::const_iterator > fixed_utilizations() const {
                return std::make_pair(m_fixed_utilization.begin(), m_fixed_utilization.end());
            }

            std::pair< std::vector<double>::const_iterator, std::vector<double>::const_iterator > free_spaces() const {
                return std::make_pair(m_free_space.begin(), m_free_space.end());
            }

            void position(entity_system::entity bin, point position);

            void dimension(entity_system::entity bin, point dimension);

            void movable_utilization(entity_system::entity bin, double movable_utilization);

            void fixed_utilization(entity_system::entity bin, double fixed_utilization);

            void free_space(entity_system::entity bin, double free_space);
        };
    }
}


#endif //ophidian_BINS_H
