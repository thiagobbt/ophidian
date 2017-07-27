#include "wrapper.h"

Wrapper::Wrapper(std::string tech_lef, std::string cell_lef, std::string input_def):
    mPlacement(mNetlist), mLibraryMapping(mNetlist), mLibrary(mStdCells), mPlacementMapping(mPlacement, mLibrary, mNetlist, mLibraryMapping)
{
    ophidian::parser::DefParser defParser;
    std::unique_ptr<ophidian::parser::Def> def = defParser.readFile(input_def);
    ophidian::placement::def2placement(*def, mPlacement, mNetlist);
    mCircuitName = def->circuitName();

    mInputDefPath = input_def;

    ophidian::parser::LefParser lefParser;
    std::unique_ptr<ophidian::parser::Lef> lef = std::make_unique<ophidian::parser::Lef>();
    lefParser.readFile(cell_lef, lef);
    lefParser.readFile(tech_lef, lef);
    ophidian::floorplan::lefDef2Floorplan(*lef, *def, mFloorplan);

    ophidian::placement::lef2Library(*lef, mLibrary, mStdCells);

    ophidian::circuit::def2LibraryMapping(*def, mNetlist, mStdCells, mLibraryMapping);
    mDistanceMicrons = def->database_units();
}

void Wrapper::writeDefFile(std::__cxx11::string filePath)
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
                for (auto cellIt = mNetlist.begin(ophidian::circuit::Cell()); cellIt != mNetlist.end(ophidian::circuit::Cell()); ++cellIt)
                {
                    defFile << "   - " << mNetlist.name(*cellIt);
                    defFile << " " << mStdCells.name(mLibraryMapping.cellStdCell(*cellIt)) << "\n";
                    defFile << "      + ";
                    if (mPlacement.isFixed(*cellIt))
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
