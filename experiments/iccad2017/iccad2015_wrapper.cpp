#include "iccad2015_wrapper.h"

iccad2015_wrapper::iccad2015_wrapper(std::__cxx11::string circuitPath, std::__cxx11::string circuitName) :
    mPlacement(mNetlist), mLibraryMapping(mNetlist), mLibrary(mStdCells), mPlacementMapping(mPlacement, mLibrary, mNetlist, mLibraryMapping), mCircuitName(circuitName)
{
    std::cout << circuitPath+ "/" + circuitName + ".def" << std::endl;

    ophidian::parser::DefParser defParser;
    std::unique_ptr<ophidian::parser::Def> def = defParser.readFile(circuitPath+ "/" + circuitName + ".def");
    ophidian::placement::def2placement(*def, mPlacement, mNetlist);

    ophidian::parser::LefParser lefParser;
    std::unique_ptr<ophidian::parser::Lef> lef =  std::make_unique<ophidian::parser::Lef>();
    lefParser.readFile(circuitPath+ "/" + circuitName + ".lef", lef);
    ophidian::floorplan::lefDef2Floorplan(*lef, *def, mFloorplan);

    ophidian::placement::lef2Library(*lef, mLibrary, mStdCells);

    ophidian::circuit::def2LibraryMapping(*def, mNetlist, mStdCells, mLibraryMapping);
    mDistanceMicrons = def->database_units();
}
