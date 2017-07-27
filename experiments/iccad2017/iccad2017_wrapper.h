#ifndef ICCAD2017_FIXTURE_H
#define ICCAD2017_FIXTURE_H

#include <string>
#include <iostream>
#include <fstream>

#include <ophidian/parser/Def.h>
#include <ophidian/parser/Lef.h>

#include <ophidian/placement/Placement.h>
#include <ophidian/placement/Library.h>
#include <ophidian/placement/PlacementMapping.h>
#include <ophidian/circuit/Netlist.h>
#include <ophidian/floorplan/Floorplan.h>
#include <ophidian/standard_cell/StandardCells.h>
#include <ophidian/circuit/LibraryMapping.h>


#include <ophidian/placement/Def2Placement.h>
#include <ophidian/placement/Lef2Library.h>
#include <ophidian/floorplan/LefDef2Floorplan.h>
#include <ophidian/circuit/Def2LibraryMapping.h>

#include <ophidian/placement/Fences.h>
#include <ophidian/placement/Def2Fence.h>

class iccad2017_wrapper
{
public:
    iccad2017_wrapper(std::string circuitPath, std::string circuitName);

    void writeDefFile(std::string filePath);

public:
    ophidian::circuit::Netlist mNetlist;
    ophidian::floorplan::Floorplan mFloorplan;
    ophidian::placement::Placement mPlacement;
    ophidian::placement::Fences mFences;

    ophidian::standard_cell::StandardCells mStdCells;
    ophidian::placement::Library mLibrary;

    ophidian::circuit::LibraryMapping mLibraryMapping;
    ophidian::placement::PlacementMapping mPlacementMapping;

    std::string mCircuitName;
    double mDistanceMicrons;

    std::string mInputDefPath;
};

#endif // ICCAD2017_FIXTURE_H
