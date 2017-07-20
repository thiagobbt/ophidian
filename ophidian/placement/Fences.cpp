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

#include "Fences.h"

namespace ophidian
{
namespace placement
{

Fences::Fences(circuit::Netlist &netlist) : fenceNames_(fences_),
                   fenceAreas_(fences_),
                   fenceCells_(fences_, netlist.cells_)
                   {}

Fences::~Fences() = default;

Fence Fences::add(const std::string &name)
{
    if (name2Fence_.find(name) == name2Fence_.end()) {
        auto fence = fences_.add();
        fenceNames_[fence] = name;
        name2Fence_[name] = fence;
        return fence;
    } else {
        return name2Fence_[name];
    }
}

void Fences::erase(const Fence &fence)
{
    name2Fence_.erase(name(fence));
    fences_.erase(fence);
}

void Fences::reserve(uint32_t size)
{
    fences_.reserve(size);
}

uint32_t Fences::size() const
{
    return fences_.size();
}

uint32_t Fences::capacity() const
{
    return fences_.capacity();
}

Fence Fences::find(std::string fenceName)
{
    return name2Fence_[fenceName];
}

std::string Fences::name(const Fence &fence) const
{
    return fenceNames_[fence];
}

util::Range<Fences::FencesIterator> Fences::range() const
{
    return util::Range<Fences::FencesIterator>(fences_.begin(), fences_.end());
}

void Fences::area(const Fence &fr, const util::MultiBox &mb)
{
    fenceAreas_[fr] = mb;
}

util::MultiBox Fences::area(const Fence &fence) const
{
    return fenceAreas_[fence];
}

Fences::Cells Fences::members(const Fence &fence) const
{
    return fenceCells_.parts(fence);
}

void Fences::connect(const Fence &fr, const circuit::Cell &c)
{
    fenceCells_.addAssociation(fr, c);
}

}
}
