#ifndef ICCAD2017_H
#define ICCAD2017_H

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

class Wrapper
{
public:
    Wrapper(std::string tech_lef, std::string cell_lef, std::string input_def);

    void writeDefFile(std::string filePath);

public:
    ophidian::circuit::Netlist mNetlist;
    ophidian::floorplan::Floorplan mFloorplan;
    ophidian::placement::Placement mPlacement;

    ophidian::standard_cell::StandardCells mStdCells;
    ophidian::placement::Library mLibrary;

    ophidian::circuit::LibraryMapping mLibraryMapping;
    ophidian::placement::PlacementMapping mPlacementMapping;

    std::string mCircuitName;
    double mDistanceMicrons;

    std::string mInputDefPath;
};

#endif // ICCAD2017_H
