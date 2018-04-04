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

#include "Def.h"
#include "ParserException.h"

#include <cstring>

namespace ophidian {
namespace parser {

std::unique_ptr<Def> DefParser::readFile(const std::string & filename) const
{
    auto def = std::make_unique<Def>();
    defrInit();

    defrSetDesignCbk([](defrCallbackType_e, const char *string, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        that.circuit_name = string;
        return 0;
    });

    defrSetUnitsCbk([](defrCallbackType_e, double number, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        that.units_ = number;
        return 0;
    });

    defrSetDieAreaCbk([](defrCallbackType_e, defiBox *box, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        that.die_.lower = {box->xl(), box->yl()};
        that.die_.upper = {box->xh(), box->yh()};
        return 0;
    });

    defrSetRowCbk([](defrCallbackType_e, defiRow *defrow, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        Def::row r;
        r.name = defrow->name();
        r.site = defrow->macro();
        r.num = {defrow->xNum(), defrow->yNum()};
        r.step = {defrow->xStep(), defrow->yStep()};
        r.origin = {defrow->x(), defrow->y()};
        that.rows_.push_back(r);
        return 0;
    });

    defrSetComponentStartCbk([](defrCallbackType_e, int number, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        that.components_.reserve(number);
        return 0;
    });

    defrSetComponentCbk([](defrCallbackType_e, defiComponent *comp, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);

        Def::component c;
        c.name = comp->id();
        c.macro = comp->name();
        c.fixed = comp->isFixed();
        c.position = {comp->placementX(), comp->placementY()};
        c.orientation = comp->placementOrientStr();
        that.components_.push_back(c);
        return 0;
    });

    defrSetRegionStartCbk([](defrCallbackType_e, int number, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        that.regions_.reserve(number);
        return 0;
    });

    defrSetRegionCbk([](defrCallbackType_e, defiRegion *reg, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);

        Def::region r;
        r.name = reg->name();
        r.rectangles.reserve(reg->numRectangles());

        for (int i = 0; i < reg->numRectangles(); i++) {
            r.rectangles.emplace_back(geometry::Point(reg->xl(i), reg->yl(i)), geometry::Point(reg->xh(i), reg->yh(i)));
        }

        that.regions_.push_back(r);
        return 0;
    });

    defrSetGroupsStartCbk([](defrCallbackType_e, int number, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        that.groups_.reserve(number);
        return 0;
    });

    defrSetGroupNameCbk([](defrCallbackType_e, const char* group_name, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);

        that.group_helper_ = group_name;
        return 0;
    });

    defrSetGroupMemberCbk([](defrCallbackType_e, const char* group_members, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);

        that.groups_[that.group_helper_].push_back(group_members);

        return 0;
    });

    defrSetNetCbk([](defrCallbackType_e, defiNet *net, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        Def::net defNet;
        defNet.name = net->name();
        defNet.pins.reserve(net->numConnections());
        for(std::size_t i = 0; i < net->numConnections(); i++){
            Def::pin pin;
            pin.name = net->pin(i);
            pin.instance = net->instance(i);
            defNet.pins.push_back(pin);
        }
        that.nets_.push_back(defNet);
        return 0;
    });

    defrSetPinCbk([](defrCallbackType_e, defiPin *pin, defiUserData ud)->int{
        Def& that = *static_cast<Def*>(ud);
        Def::pin defPin;
        defPin.name = pin->pinName();

        if (pin->hasDirection()) {
            if (strcmp(pin->direction(), "INPUT") == 0) {
                defPin.direction = Def::direction_t::INPUT;
            } else if (strcmp(pin->direction(), "OUTPUT") == 0) {
                defPin.direction = Def::direction_t::OUTPUT;
            } else if (strcmp(pin->direction(), "INOUT") == 0) {
                defPin.direction = Def::direction_t::INOUT;
            } else if (strcmp(pin->direction(), "FEEDTHRU") == 0) {
                defPin.direction = Def::direction_t::FEEDTHRU;
            } else {
                defPin.direction = Def::direction_t::NA;
            }
        }

        if (pin->hasPlacement()) {
            defPin.placement = {pin->placementX(), pin->placementY()};
        }

        that.pins_.push_back(defPin);
        return 0;
    });

    FILE* ifp = fopen(filename.c_str(), "r");
    if(ifp){
        auto res = defrRead(ifp, filename.c_str(), def.get(), true);
    } else {
        throw InexistentFile(filename);
    }

    fclose(ifp);
    defrClear();

    return def;
}

DefParser::DefParser()
{ }
DefParser::~DefParser()
{ }

Def::Def()
{ }
Def::~Def()
{ }

}
}
