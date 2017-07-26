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

#ifndef OPHIDIAN_PLACEMENT_FENCES_H
#define OPHIDIAN_PLACEMENT_FENCES_H

#include <ophidian/circuit/Netlist.h>
#include <ophidian/entity_system/Property.h>
#include <ophidian/parser/Def.h>
#include <ophidian/util/Range.h>
#include <ophidian/util/Units.h>
#include <string>

namespace ophidian
{
namespace placement
{

class Fence : public entity_system::EntityBase
{
public:
    using entity_system::EntityBase::EntityBase;
};

class FenceArea : public entity_system::EntityBase
{
public:
    using entity_system::EntityBase::EntityBase;
};

class Fences
{
public:
    using FencesIterator = entity_system::EntitySystem<Fence>::const_iterator;
    using FenceAreasIterator = entity_system::EntitySystem<FenceArea>::const_iterator;
    using Cells = entity_system::Association<Fence, circuit::Cell>::Parts;

    Fences(circuit::Netlist &netlist);

    ~Fences();

    //! Add Fence
    /*!
      \brief Adds a fence instance. A fence has a name associated to it. If the
             fence already exist then just return the existing fence.
      \param name Name of the fence, used to identify it.
      \return A handler for the created/existing Fence.
    */
    Fence add(const std::string &name);

    //! Erase Fence
    /*!
      \param fence A handler for the Fence to erase.
      \brief Erases a Fence instance.
    */
    void erase(const Fence &fence);

    //! Allocate space for storing Fence entities
    /*!
      \brief Using this function, it is possible to avoid superfluous memory
             allocation: if you know that the netlist you want to build will be
             large (e.g. it will contain millions fences), then it is worth
             reserving space for this amount before starting to build the
             standard fences.
      \param size Minimum capacity for the fence container.
    */
    void reserve(uint32_t size);

    //! Size of Fence's System
    /*!
      \brief Returns the number of Fences.
      \return The number of Fences.
    */
    uint32_t size() const;

    //! Capacity of the Fence's System
    /*!
      \return The capacity of the Fence EntitySystem.
    */
    uint32_t capacity() const;

    //! Find a fence
    /*!
      \brief Using the mapping, return a fence handler by fence's name.
      \param The fence name.
      \return Return a fence handler by fence's name.
    */
    Fence find(std::string fenceName);

    //! Fences iterator
    /*!
       \return Range iterator for the fences.
     */
    util::Range<Fences::FencesIterator> range() const;

    //! Fence name getter
    /*!
       \brief Get the name of a given fence.
       \param fence Fence entity to get the name.
       \return Name of the fence
     */
    std::string name(const Fence &fence) const;

    //! Make Fence Property
    /*!
       \brief Creates a Property for the Fence's Entity System.
       \tparam Value value type of the Property.
       \return An Fence => \p Value Map.
    */
    template <typename Value>
    entity_system::Property<Fence, Value> makeProperty()
    const {
        return entity_system::Property<Fence, Value>(fences_);
    }

    //! Set Fence geometry
    /*!
       \brief Defines the area of the fence.
       \param fr A handler for the Fence to set the area.
       \param mb A \p MultiBox representing the area of the fence.
    */
    void area(const Fence &fr, const util::MultiBox &mb);

    //! Fence geometry getter
    /*!
       \brief Get the area of a given fence.
       \param fence Fence entity to get the name.
       \return Area of the fence
     */
    util::MultiBox area(const Fence &fence) const;

    //! Fence members getter
    /*!
       \brief Get the member components of a given fence.
       \param fence Fence entity to get the name.
       \return \p Cells which can be iterated through.
     */
    Cells members(const Fence &fence) const;

    //! Fence members adder
    /*!
       \brief Defines a \p Cell as a member of the \p Fence.
       \param fr A handler for the Fence.
       \param c A handler for the cell.
     */
    void connect(const Fence &fr, const circuit::Cell &c);

private:
    entity_system::EntitySystem<Fence> fences_;

    entity_system::Property<Fence, std::string> fenceNames_;
    entity_system::Property<Fence, util::MultiBox> fenceAreas_;

    entity_system::Aggregation<Fence, circuit::Cell> fenceCells_;

    std::unordered_map<std::string, Fence> name2Fence_;
};
} // namespace placement
} // namespace ophidian

#endif // OPHIDIAN_PLACEMENT_FENCES_H
