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

#include "lef.h"
#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <iostream>
namespace ophidian {
namespace parsing {

lef::lef(const std::string &filename)
{
    lefrInit();

    lefrSetUnitsCbk([](lefrCallbackType_e,
                    lefiUnits* units,
                    lefiUserData ud)->int{
        static_cast<lef*>(ud)->m_units = *units;
        static_cast<lef*>(ud)->m_units.setDatabase(units->databaseName(), units->databaseNumber());
        return 0;
    });

    lefrSetSiteCbk([](lefrCallbackType_e,
                   lefiSite* l,
                   lefiUserData ud)->int{
        site s;
        s.name = l->name();
        s.class_ = (l->hasClass() ? l->siteClass() : "");
        if(l->hasXSymmetry())
            s.setXsymmetry();
        if(l->hasYSymmetry())
            s.setYsymmetry();
        if(l->has90Symmetry())
            s.set90symmetry();
        s.x = l->sizeX();
        s.y = l->sizeY();
        static_cast<lef*>(ud)->m_sites.push_back(s);
        return 0;
    });

    lefrSetLayerCbk([](lefrCallbackType_e,
                    lefiLayer* l,
                    lefiUserData ud)->int{
        layer lay;
        lay.name = l->name();
        lay.type = (l->hasType()?l->type():"");
        lay.direction = layer::NOT_ASSIGNED;
        if(l->hasDirection())
        {
            if(std::string(l->direction()) == "HORIZONTAL" ||
                    std::string(l->direction()) == "horizontal")
                lay.direction = layer::HORIZONTAL;
            else if(std::string(l->direction()) == "VERTICAL" ||
                    std::string(l->direction()) == "vertical")
                lay.direction = layer::VERTICAL;
        }
        lay.pitch = l->pitch();
        lay.width = l->width();
        static_cast<lef*>(ud)->m_layers.push_back(lay);
        return 0;
    });
    lefrSetPinCbk([](lefrCallbackType_e,
                  lefiPin* l,
                  lefiUserData ud)->int{
        macro & m = static_cast<lef*>(ud)->m_macros.back();
        pin p;
        p.name = l->name();
        if(l->hasDirection())
        {
            if(std::string(l->direction()) == "INPUT")
                p.direction = pin::INPUT;
            else if(std::string(l->direction()) == "OUTPUT")
                p.direction  = pin::OUTPUT;
        }
        for(int i = 0; i < l->numPorts(); ++i)
        {
            port pt;
            for(int j = 0; j < l->port(i)->numItems(); ++j)
            {
                switch(l->port(i)->itemType(j)) {
                case lefiGeomLayerE:
                    pt.layers.push_back(l->port(i)->getLayer(j));
                    break;
                case lefiGeomRectE:
                    rect r;
                    r.xl = l->port(i)->getRect(j)->xl;
                    r.yl = l->port(i)->getRect(j)->yl;
                    r.xh = l->port(i)->getRect(j)->xh;
                    r.yh = l->port(i)->getRect(j)->yh;
                    pt.rects.push_back(r);
                    break;
                }
            }
            p.ports.push_back(pt);
        }
        m.pins.push_back(p);
        return 0;
    });

    lefrSetMacroBeginCbk([](lefrCallbackType_e,
                         const char *string,
                         lefiUserData ud)->int{
        static_cast<lef*>(ud)->m_macros.push_back(macro{string});
        return 0;
    });

    lefrSetObstructionCbk([](lefrCallbackType_e,
                          lefiObstruction* l,
                          lefiUserData ud)->int{
        auto geometries = l->geometries();
        macro & m = static_cast<lef*>(ud)->m_macros.back();
        std::string last_layer;
        for(int i = 0; i < geometries->numItems(); ++i)
        {
            switch(geometries->itemType(i))
            {
            case lefiGeomLayerE:
                last_layer = geometries->getLayer(i);
                break;
            case lefiGeomRectE:
                auto geom_rect =  geometries->getRect(i);
                rect r{geom_rect->xl, geom_rect->yl, geom_rect->xh, geom_rect->yh};
                m.obses.layer2rects[last_layer].push_back(r);
                break;
            }
        }
        return 0;
    });

    lefrSetMacroCbk([](lefrCallbackType_e,
                    lefiMacro* l,
                    lefiUserData ud)->int{

        macro & m = static_cast<lef*>(ud)->m_macros.back();
        m.name = l->name();
        m.class_ = (l->hasClass()?l->macroClass():"");
        m.origin.x = l->originX();
        m.origin.y = l->originY();
        if(l->hasForeign())
        {
            m.foreign.name = l->foreignName();
            m.foreign.x = l->foreignX();
            m.foreign.y = l->foreignY();
        }
        m.size = macro_size{l->sizeX(), l->sizeY()};
        m.site = (l->hasSiteName()?l->siteName():"");
        return 0;
    });


    lefrSetMacroEndCbk([](lefrCallbackType_e,
                       const char *string,
                       lefiUserData)->int{
        return 0;
    });


    FILE* ifp = fopen(filename.c_str(), "r");
    auto res = lefrRead(ifp, filename.c_str(), this);

    if(res)
        std::cerr << "error" << std::endl;

    fclose(ifp);

}

lef::~lef()
{

}

void lef::site::setXsymmetry()
{
    symmetry |= X;
}

void lef::site::setYsymmetry()
{
    symmetry |= Y;
}

void lef::site::set90symmetry()
{
    symmetry |= NINETY;
}

} /* namespace placement */
} /* namespace ophidian */

