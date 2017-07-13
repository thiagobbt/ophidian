#include "iccad2017_wrapper.h"

iccad2017_wrapper::iccad2017_wrapper(std::__cxx11::string circuitPath, std::__cxx11::string circuitName) :
    mPlacement(mNetlist), mLibraryMapping(mNetlist), mLibrary(mStdCells), mPlacementMapping(mPlacement, mLibrary, mNetlist, mLibraryMapping), mCircuitName(circuitName)
{
    ophidian::parser::DefParser defParser;
    std::unique_ptr<ophidian::parser::Def> def = defParser.readFile(circuitPath+"/placed.def");
    ophidian::placement::def2placement(*def, mPlacement, mNetlist);

    ophidian::parser::LefParser lefParser;
    std::unique_ptr<ophidian::parser::Lef> lef =  std::make_unique<ophidian::parser::Lef>();
    lefParser.readFile(circuitPath + "/cells_modified.lef", lef);
    lefParser.readFile(circuitPath + "/tech.lef", lef);
    ophidian::floorplan::lefDef2Floorplan(*lef, *def, mFloorplan);

    ophidian::placement::lef2Library(*lef, mLibrary, mStdCells);

    ophidian::circuit::def2LibraryMapping(*def, mNetlist, mStdCells, mLibraryMapping);
    mDistanceMicrons = def->database_units();

}

void iccad2017_wrapper::writeDefFile(std::__cxx11::string filePath)
{
    std::ofstream defFile;
    defFile.open (filePath);
    defFile << "\n";
    defFile << "VERSION 5.8 ;\n";
    defFile << "DIVIDERCHAR \"/\" ;\n";
    defFile << "BUSBITCHARS \"[]\" ;\n";
    defFile << "DESIGN " << mCircuitName << " ;\n";
    defFile << "UNITS DISTANCE MICRONS "<< (int)mDistanceMicrons <<" ;\n";
    defFile << "\n";
    defFile << "DIEAREA ( "
            << units::unit_cast<int>(mFloorplan.chipOrigin().x()) << " "
            << units::unit_cast<int>(mFloorplan.chipOrigin().y()) << " ) ( "
            << units::unit_cast<int>(mFloorplan.chipUpperRightCorner().x()) << " "
            << units::unit_cast<int>(mFloorplan.chipUpperRightCorner().y()) << " ) ;\n";

    defFile << "\n";
    defFile << "\n";
    defFile << "COMPONENTS "<< mNetlist.size(ophidian::circuit::Cell()) <<" ;\n";
    for(auto cellIt = mNetlist.begin(ophidian::circuit::Cell()); cellIt != mNetlist.end(ophidian::circuit::Cell()); ++cellIt)
    {
        defFile << "   - " << mNetlist.name(*cellIt);
        defFile << " " << mStdCells.name(mLibraryMapping.cellStdCell(*cellIt))<<"\n";
        defFile << "      + ";
        if(mPlacement.isFixed(*cellIt))
        {
            defFile << "FIXED ( ";
        }
        else {
            defFile << "PLACED ( ";
        }
        defFile << units::unit_cast<int>(mPlacement.cellLocation(*cellIt).x()) << " ";
        defFile << units::unit_cast<int>(mPlacement.cellLocation(*cellIt).y()) << " ) ";
        defFile << mPlacement.cellOrientation(*cellIt) << " ;\n";
    }
    defFile << "END COMPONENTS\n";
    defFile << "\n";
    defFile.close();
}
