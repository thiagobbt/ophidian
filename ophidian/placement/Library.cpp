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

#include "Library.h"

namespace ophidian
{
namespace placement
{

Library::Library(const standard_cell::StandardCells &std_cells) :
    geometries_(std_cells.makeProperty<util::MultiBox>(standard_cell::Cell())),
    pinOffsets_(std_cells.makeProperty<util::Location>(standard_cell::Pin())),
    alignment_(std_cells.makeProperty<RowAlignment>(standard_cell::Cell()))
{
}

void Library::geometry(const standard_cell::Cell &cell, const util::MultiBox &geometry)
{
    geometries_[cell] = geometry;
}

void Library::pinOffset(const standard_cell::Pin &pin, const util::Location &offset)
{
    pinOffsets_[pin] = offset;
}

void Library::cellAlignment(const standard_cell::Cell &cell, const RowAlignment alignment)
{
    alignment_[cell] = alignment;
}


} // namespace placement
} // namespace ophidian
