#include "Def2LibraryMapping.h"

namespace ophidian
{
namespace circuit
{

void def2LibraryMapping(const parser::Def & def, circuit::Netlist & netlist, standard_cell::StandardCells & standardCells, LibraryMapping & libraryMapping)
{
    for(auto & component : def.components())
    {
        auto cell = netlist.add(Cell(), component.name);
        auto stdCell = standardCells.add(standard_cell::Cell(), component.macro);
        libraryMapping.cellStdCell(cell, stdCell);

        for (auto pin : netlist.pins(cell)) {
            std::string fullPinName = netlist.name(pin);
            int basePinNameOffset = fullPinName.find(":");
            std::string basePinName = fullPinName.substr(basePinNameOffset + 1);
            std::string stdCellPinName = component.macro + ":" + basePinName;

            auto stdCellPin = standardCells.find(standard_cell::Pin(), stdCellPinName);
            libraryMapping.pinStdCell(pin, stdCellPin);
        }
    }
}

} // namespace circuit
} // namespace ophidian
