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

#include "Design.h"

#include <string>
#include <iostream>
#include <fstream>

namespace ophidian
{

namespace design
{

Design::Design() :

	mNetlist(),
	mFloorplan(),
	mPlacement(mNetlist),
	mStandardCells(),
	mLibrary(mStandardCells),
    mLibraryMapping(mNetlist),
    mPlacementMapping(mPlacement, mLibrary, mNetlist, mLibraryMapping),
    mFences(mNetlist)
{

}

Design::~Design()
{

}

void Design::writeDefFile(std::__cxx11::string filePath)
{
    std::ifstream inputDef(mInputDefPath);

    std::ofstream defFile;
    defFile.open (filePath);

    bool foundComponents = false;
    std::string line;
    while (std::getline(inputDef, line))
    {
        if (foundComponents)
        {
            if (line.substr(0, 3) == "END")
            {
                defFile << "COMPONENTS "<< mNetlist.size(ophidian::circuit::Cell()) << " ;\n";
//                for (auto cellIt = mNetlist.begin(ophidian::circuit::Cell()); cellIt != mNetlist.end(ophidian::circuit::Cell()); ++cellIt)
                for (auto cell : mFences.members(mFences.find("er3")))
                {
//                    auto cell = *cellIt;
                    defFile << "   - " << mNetlist.name(cell);
                    defFile << " " << mStandardCells.name(mLibraryMapping.cellStdCell(cell)) << "\n";
                    defFile << "      + ";
                    if (mPlacement.isFixed(cell))
                    {
                        defFile << "FIXED ( ";
                    }
                    else {
                        defFile << "PLACED ( ";
                    }
                    defFile << units::unit_cast<int>(mPlacement.cellLocation(cell).x()) << " ";
                    defFile << units::unit_cast<int>(mPlacement.cellLocation(cell).y()) << " ) ";
                    defFile << mPlacement.cellOrientation(cell) << " ;\n";
                }
                defFile << "END COMPONENTS\n";
                defFile << "\n";

                foundComponents = false;
            }
            else {
                // Found components and not in end, skip
            }
        }
        else if (line.substr(0, 10) == "COMPONENTS")
        {
            foundComponents = true;
        }
        else {
            defFile << line << std::endl;
        }
    }

    inputDef.close();
    defFile.close();
}

} //namespace design

} //namespace ophidian
