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

namespace ophidian {
namespace parser {

std::unique_ptr<Def> DefParser::readFile(const std::string & filename) const throw(InexistentFile)
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

    FILE* ifp = fopen(filename.c_str(), "r");
    if(ifp){
        auto res = defrRead(ifp, filename.c_str(), def.get(), true);
    } else {
        throw InexistentFile();
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
