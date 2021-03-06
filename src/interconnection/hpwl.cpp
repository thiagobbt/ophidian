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

#include "hpwl.h"

namespace ophidian {
    namespace interconnection {
        double hpwl(const std::vector<geometry::point<double> >& points) {
            geometry::point<double> lower( std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
            geometry::point<double> upper( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() );
            for(auto & p : points)
            {
                lower.x(std::min(lower.x(), p.x()));
                lower.y(std::min(lower.y(), p.y()));
                upper.x(std::max(upper.x(), p.x()));
                upper.y(std::max(upper.y(), p.y()));
            }
            return ophidian::geometry::manhattan_distance(lower, upper);
        }
    }
}
