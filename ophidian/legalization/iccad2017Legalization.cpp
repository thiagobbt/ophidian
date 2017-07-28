#include "iccad2017Legalization.h"

#include <boost/lexical_cast.hpp>

namespace ophidian
{
namespace legalization
{
iccad2017Legalization::iccad2017Legalization(design::Design &design)
    : mDesign(design), mMultirowAbacus(design.netlist(), design.floorplan(), design.placement(), design.placementMapping())
{

}

using boost::lexical_cast;
void iccad2017Legalization::initializeTemporaryBlocs()
{
    mTemporaryBlocs.reserve(mDesign.fences().size() * 5);
    for(auto fence : mDesign.fences().range())
    {
        util::MultiBox boxes = mDesign.fences().area(fence);
        int contBox = 0;
        for(auto box : boxes)
        {
            std::string cellName = mDesign.fences().name(fence) + "Block"+ lexical_cast<std::string>(contBox);
            auto stdCell = mDesign.standardCells().add(ophidian::standard_cell::Cell(), cellName);
            auto circuitCell = mDesign.netlist().add(ophidian::circuit::Cell(), cellName);

            geometry::Box traslatedBox;
            geometry::translate(box, -box.min_corner().x(), -box.min_corner().y(), traslatedBox);

            mDesign.library().geometry(stdCell, util::MultiBox({traslatedBox}));
            mDesign.library().cellAlignment(stdCell, placement::RowAlignment::NA);
            mDesign.placement().placeCell(circuitCell, util::Location(box.min_corner().x(), box.min_corner().y()));
            mDesign.placement().fixLocation(circuitCell, true);
            mDesign.libraryMapping().cellStdCell(circuitCell, stdCell);

            mTemporaryBlocs.push_back(circuitCell);

            contBox++;
        }
    }
}

void iccad2017Legalization::eraseTemporaryBlocs()
{
    for(auto circuitCell : mTemporaryBlocs)
    {
        mDesign.netlist().erase(circuitCell);
    }
}

void iccad2017Legalization::fixFencesCells(bool fix)
{
    for(auto fence : mDesign.fences().range())
    {
        for(circuit::Cell cell : mDesign.fences().members(fence))
        {
            mDesign.placement().fixLocation(cell, fix);
        }
    }
}

void iccad2017Legalization::legalize()
{
    //posiciona fences (paralelo)
    for(auto fence : mDesign.fences().range())
    {
        std::vector<circuit::Cell> cells (mDesign.fences().members(fence).begin(), mDesign.fences().members(fence).end());
        mMultirowAbacus.legalizePlacement(cells, mDesign.fences().area(fence));
    }

    //fixa celulas das fences
    fixFencesCells(true);
    //cria blocos das fences
    initializeTemporaryBlocs();
    //posiciona circuito
    std::vector<circuit::Cell> cells;
    cells.reserve(mDesign.netlist().size(circuit::Cell()));
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
    {
        if(!mDesign.placement().cellHasFence(*cellIt))
        {
            cells.push_back(*cellIt);
        }
    }
    geometry::Box chipArea(mDesign.floorplan().chipOrigin().toPoint(), mDesign.floorplan().chipUpperRightCorner().toPoint());
    util::MultiBox legalizationArea({chipArea});
    mMultirowAbacus.legalizePlacement(cells, legalizationArea);

    //deleta blocos das fences
    eraseTemporaryBlocs();

    //desfixa celulas das fences
    fixFencesCells(false);
}

} // namespace legalization
} // namespace ophidian
