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

#ifndef OPHIDIAN_UTIL_UNITS_H
#define OPHIDIAN_UTIL_UNITS_H

#include <algorithm>
#include <ratio>
#include <vector>
#include <3rdparty/units/include/units.h>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry.hpp>

#include <ophidian/geometry/Models.h>
#include <ophidian/geometry/Operations.h>

namespace ophidian {
namespace util {

//!Length Units
using picometer_t   = units::length::picometer_t;
using nanometer_t   = units::length::nanometer_t;
using micrometer_t  = units::length::micrometer_t;
using millimeter_t  = units::length::millimeter_t;
using meter_t       = units::length::meter_t;

//!Lenght Unit derived from micrometer to be used as internal length unit length (as commonly used in LEF/DEF library)
//!To convert to/from micron, please use the DbuConverter Utility class
using dbu_base = units::unit<std::ratio<1>, units::length::micrometers>;
using dbumeter_t = units::unit_t<dbu_base, double, units::linear_scale>;

//!Area Units
//!New unit tag derived from area_unit to allow for creation of square_millimiter_t
using square_millimeter = units::unit<std::micro, units::category::area_unit>;

using square_millimeter_t = units::unit_t<square_millimeter>;
using square_meter_t = units::area::square_meter_t;

//!Time Units
using picosecond_t  = units::time::picosecond_t;
using nanosecond_t  = units::time::nanosecond_t;
using microsecond_t = units::time::microsecond_t;
using millisecond_t  = units::time::millisecond_t;
using second_t      = units::time::second_t;

//!Capacitance Units
using femtofarad_t = units::capacitance::femtofarad_t;
using picofarad_t  = units::capacitance::picofarad_t;
using farad_t      = units::capacitance::farad_t;

//!Resistance Units
using ohm_t       = units::impedance::ohm_t;
using kiloohm_t   = units::impedance::kiloohm_t;

} //namespace util

} //namespace ophidian


//Creating a new boost point to allow receiving double in the constructor and converting to micrometer_t
namespace boost {
namespace geometry {
namespace model {
namespace d2 {
template<typename CoordinateType, typename CoordinateSystem = cs::cartesian>
class point_xy_location : public model::point<CoordinateType, 2, CoordinateSystem>
{
public:

#ifndef BOOST_NO_CXX11_DEFAULTED_FUNCTIONS
    /// \constructor_default_no_init
    point_xy_location() = default;
#else
    /// \constructor_default_no_init
    inline point_xy()
    {}
#endif

    /// Constructor with x/y values as double with implicit convertion to micrometer_t
    inline point_xy_location(double const& x, double const& y)
        : model::point<CoordinateType, 2, CoordinateSystem>(CoordinateType(x), CoordinateType(y))
    {}

    /// Constructor with x/y values
    inline point_xy_location(CoordinateType const& x, CoordinateType const& y)
        : model::point<CoordinateType, 2, CoordinateSystem>(x, y)
    {}

    /// Get x-value
    inline CoordinateType const& x() const
    { return this->template get<0>(); }

    /// Get y-value
    inline CoordinateType const& y() const
    { return this->template get<1>(); }

    /// Set x-value
    inline void x(CoordinateType const& v)
    { this->template set<0>(v); }

    /// Set y-value
    inline void y(CoordinateType const& v)
    { this->template set<1>(v); }

    ///operator == overloading
    bool operator==(const point_xy_location & point)
    { return (this->x() == point.x()) && (this->y() == point.y());}

    ///operator == overloading
    bool operator!=(const point_xy_location & point)
    { return !(*this==point);}

    ophidian::geometry::Point toPoint() {
            return ophidian::geometry::Point(units::unit_cast<double>(this->x()), units::unit_cast<double>(this->y()));
        }
};
}
}
}
}

namespace ophidian {
namespace util {

using Location = boost::geometry::model::d2::point_xy_location<ophidian::util::micrometer_t>;

class MultiBox {
public:
    MultiBox() {

    }

    MultiBox(const std::vector<geometry::Box> & boxes)
        : boxes_(boxes) {

    }

    MultiBox(const MultiBox & otherBox)
        : boxes_(otherBox.boxes_) {

    }

    void push_back(const geometry::Box & box) {
        boxes_.push_back(box);
    }

    std::vector<geometry::Box>::iterator begin() {
        return boxes_.begin();
    }

    std::vector<geometry::Box>::iterator end() {
        return boxes_.end();
    }

    std::vector<geometry::Box>::const_iterator begin() const {
        return boxes_.begin();
    }

    std::vector<geometry::Box>::const_iterator end() const {
        return boxes_.end();
    }

    const geometry::Box & operator[](std::size_t index) const {
        return boxes_[index];
    }

    std::size_t size() {
        return boxes_.size();
    }

    bool operator==(const MultiBox & other) const {
        return std::is_permutation(this->boxes_.begin(), this->boxes_.end(), other.boxes_.begin(),
            [](const geometry::Box &a, const geometry::Box &b) {
                return boost::geometry::equals(a, b);
            }
        );
    }

    bool operator!=(const MultiBox & other) const {
        return !(*this==other);
    }

    MultiBox translate(double xTranslation, double yTranslation) {
        std::vector<geometry::Box> translatedBoxes;
        translatedBoxes.reserve(boxes_.size());
        for (auto box : boxes_) {
            geometry::Box translatedBox;
            geometry::translate(box, xTranslation, yTranslation, translatedBox);
            translatedBoxes.push_back(translatedBox);
        }
        return MultiBox(translatedBoxes);
    }

    geometry::MultiPolygon toMultiPolygon() {
        geometry::MultiPolygon multiPolygon;
        for (auto box : boxes_) {
            geometry::MultiPolygon result;
            boost::geometry::union_(box, multiPolygon, result);
            multiPolygon = result;
        }
        return multiPolygon;
    }

private:
    std::vector<geometry::Box> boxes_;
};

} //namespace util

} //namespace ophidian

#endif // OPHIDIAN_UTIL_UNITS_H
