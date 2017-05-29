#include "iccad2017_wrapper.h"

iccad2017_wrapper::iccad2017_wrapper(std::__cxx11::string circuit) :
    mPlacement(mNetlist), mLibraryMapping(mNetlist), mLibrary(mStdCells), mPlacementMapping(mPlacement, mLibrary, mNetlist, mLibraryMapping)
{
    ophidian::parser::DefParser defParser;
    std::unique_ptr<ophidian::parser::Def> def = defParser.readFile(circuit+"/placed.def");
    ophidian::placement::def2placement(*def, mPlacement, mNetlist);

    ophidian::parser::LefParser lefParser;
    std::unique_ptr<ophidian::parser::Lef> lef =  std::make_unique<ophidian::parser::Lef>();
    lefParser.readFile(circuit + "/cells_modified.lef", lef);
    lefParser.readFile(circuit + "/tech.lef", lef);
    ophidian::floorplan::lefDef2Floorplan(*lef, *def, mFloorplan);

    ophidian::placement::lef2Library(*lef, mLibrary, mStdCells);

    ophidian::circuit::def2LibraryMapping(*def, mNetlist, mStdCells, mLibraryMapping);
}
