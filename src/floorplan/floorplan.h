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

#ifndef ophidian_FLOORPLAN_H
#define ophidian_FLOORPLAN_H

#include <boost/geometry/index/rtree.hpp>
#include "../entity_system//entity_system.h"
#include "rows.h"
#include "sites.h"
#include <unordered_map>

namespace ophidian {
    /// Namespace describing floorplan entities and basic floorplan interface.
    namespace floorplan {
        /// Floorplan class.
        /**
         * This class provides the basic floorplan interface, such as site and rows insertion, site and rows properties and circuit dimensions.
         */
        class floorplan {
            using point = geometry::point<double>;
            using box = geometry::box<point>;
            using rtree_node = std::pair<box, entity_system::entity>;
            using rtree = boost::geometry::index::rtree<rtree_node, boost::geometry::index::rstar<16>>;

            entity_system::entity_system m_sites_system;
            entity_system::entity_system m_rows_system;

            sites m_sites;
            rows m_rows;

            point m_chip_origin;
            point m_chip_boundaries;

            std::unordered_map<std::string, entity_system::entity> m_name2site;
            rtree m_rows_rtree;

        public:
            floorplan();

            point chip_origin() const {
                return m_chip_origin;
            }
            void chip_origin(point chip_origin);
/// Chip boundaries getter.
            /**
             * Returns the boundaries of the chip.
             * \return Point describing the chip boundaries.
             */
            point chip_boundaries() const {
                return m_chip_boundaries;
            }
            void chip_boundaries(point chip_boundaries);

            // sites
            /// Inserts a new site.
            /**
             * Inserts a new site in the floorplan. A site has a name and dimensions associated to it.
             * \param name Name of the site, used to identify it.
             * \param dimensions Point describing the site dimensions.
             * \return The created site.
             */
            entity_system::entity site_insert(std::string name, point dimensions);
            /// Destroys a site.
            /**
             * Destroys an existing site.
             * \param site Site to be destroyed.
             */
            void site_destroy(entity_system::entity site);

            /// Returns the number of sites.
            /**
             * Returns the number of sites created in the sites system.
             * \return Number of sites.
             */
            std::size_t site_count() const {
                return m_sites_system.size();
            }

            /// Site name getter.
            /**
             * Returns the name of a site.
             * \param site Site entity to get the name.
             * \return Name of the site.
             */
            std::string site_name(entity_system::entity site) const {
                return m_sites.name(site);
            }
            /// Site dimensions getter.
            /**
             * Returns the dimensions of a site.
             * \param site Site entity to get the dimensions.
             * \return Point describing the site dimensions.
             */
            point site_dimensions(entity_system::entity site) const {
                return m_sites.dimensions(site);
            }
            /// Sites properties getter.
            /**
             * Returns the object describing sites properties.
             * \return A constant reference to the object describing sites properties.
             */
            const sites & sites_properties() const {
                return m_sites;
            }

            // rows
            /// Inserts a new row.
            /**
             * Inserts a new row in the floorplan. A row has a site, number of sites and origin.
             * \param site Entity of the site of this row.
             * \param number_of_sites Number of sites which the row consists of.
             * \param origin Point describing the row origin.
             * \return The created row.
             */
            entity_system::entity row_insert(entity_system::entity site, unsigned number_of_sites, point origin);
            entity_system::entity row_insert(std::string site, unsigned number_of_sites, point origin);

            /// Destroys a row.
            /**
             * Destroys an existing row.
             * \param row Row to be destroyed.
             */
            void row_destroy(entity_system::entity row);

            /// Finds a row.
            /**
              Finds the row that contains a given point.
              \param point Point to query for the row.
              \return The row containing the point
             */
            entity_system::entity find_row(point point);

            /// Finds closest row to a point.
            /**
              Finds the row that is the closest to a given point.
              \param point Point to query for the row.
              \return The closest row to that point
             */
            entity_system::entity find_closest_row(point point) const;

            /// Returns the number of rows.
            /**
             * Returns the number of rows created in the rows system.
             * \return Number of rows.
             */
            std::size_t row_count() const {
                return m_rows_system.size();
            }

            /// Row site getter.
            /**
             * Returns the site of a row.
             * \param row Row entity to get the site.
             * \return Entity representing the site of the row.
             */
            entity_system::entity row_site(entity_system::entity row) const {
                return m_rows.site(row);
            }
            /// Row number of sites getter.
            /**
             * Returns the number of sites of a row.
             * \param row Row entity to get the number of sites.
             * \return Number of sites in that row.
             */
            unsigned row_number_of_sites(entity_system::entity row) const {
                return m_rows.number_of_sites(row);
            }
            /// Row origin getter.
            /**
             * Returns the origin of a row.
             * \param row Row entity to get the origin.
             * \return Point describing the origin of that row.
             */
            point row_origin(entity_system::entity row) const {
                return m_rows.origin(row);
            }
            /// Row dimensions getter.
            /**
             * Returns the dimensions of a row, calculated using the number of sites and site dimensions in that row.
             * \param row Row entity to gets the dimensions.
             * \return Point describing the dimensions of that row.
             */
            point row_dimensions(entity_system::entity row) const;
            /// Rows properties getter.
            /**
             * Returns the object describing rows properties.
             * \return A constant reference to the object describing rows properties.
             */
            const rows & rows_properties() const {
                return m_rows;
            }

            const entity_system::entity_system & rows_system() const {
                return m_rows_system;
            };
        };

        class row_not_found : public std::exception {
        public:
            const char * what() const throw() {
                return "No row found in the given coordinate";
            }
        };
    }
}



#endif //ophidian_FLOORPLAN_H
