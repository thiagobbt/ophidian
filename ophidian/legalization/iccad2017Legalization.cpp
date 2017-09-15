#include "iccad2017Legalization.h"

#include <boost/lexical_cast.hpp>

namespace ophidian
{
namespace legalization
{
iccad2017Legalization::iccad2017Legalization(design::Design &design)
    : mDesign(design),
      mCellsInitialLocations(design.netlist().makeProperty<util::Location>(circuit::Cell())),
      mMultirowAbacus(design.netlist(), design.floorplan(), design.placement(), design.placementMapping()),
      mRtreeLegalization(design)
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

void iccad2017Legalization::flipCells()
{
    auto siteHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).y();
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
    {
        if (!mDesign.placement().isFixed(*cellIt)) {
            auto cellGeometry = mDesign.placementMapping().geometry(*cellIt);
            auto cellHeight = ophidian::util::micrometer_t(cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y());
            if(std::fmod((cellHeight/siteHeight), 2.0))
            {
                //Odd-sized cells
                auto cellPosition = mDesign.placement().cellLocation(*cellIt).y();
                if(std::fmod((cellPosition/siteHeight), 2.0))
                {
                    //placed in odd line -> flip cell
                    mDesign.placement().cellOrientation(*cellIt, "S");
                }
            }
        }
    }
}

bool iccad2017Legalization::cellIsLegalized(circuit::Cell cell)
{
    if (mDesign.placement().isFixed(cell)) {
        return true;
    }

    auto siteWidth = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).x();
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).y();
    auto cellLocation = mDesign.placement().cellLocation(cell);

    bool siteAlignment = std::remainder(cellLocation.toPoint().x(), units::unit_cast<double>(siteWidth)) <= std::numeric_limits<double>::epsilon();
    bool rowAlignment = std::remainder(cellLocation.toPoint().y(), units::unit_cast<double>(rowHeight)) <= std::numeric_limits<double>::epsilon();

    return siteAlignment && rowAlignment;
}

void iccad2017Legalization::legalize()
{
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
        mCellsInitialLocations[*cellIt] = mDesign.placement().cellLocation(*cellIt);
    }

    //posiciona fences (paralelo)
    unsigned cellId = 0;
    for(auto fenceIt = mDesign.fences().range().begin(); fenceIt < mDesign.fences().range().end(); fenceIt++)
    {
        auto fence = *fenceIt;
        std::vector<circuit::Cell> cells (mDesign.fences().members(fence).begin(), mDesign.fences().members(fence).end());
        mMultirowAbacus.legalizePlacement(cells, mDesign.fences().area(fence));

//        mRtreeLegalization.buildRtree(cells, mDesign.fences().area(fence));

//        for (auto cell : cells) {
//            if (!cellIsLegalized(cell)) {
//               mRtreeLegalization.legalizeCell(cell);
//            }
//        }
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

//    mRtreeLegalization.buildRtree(cells, legalizationArea);
//    for (auto cell : cells) {
//        if (!cellIsLegalized(cell)) {
//            mRtreeLegalization.legalizeCell(cell);
//        }
//    }

    //deleta blocos das fences
    eraseTemporaryBlocs();

    //desfixa celulas das fences
    fixFencesCells(false);

    //flip cells
    flipCells();
}

} // namespace legalization
} // namespace ophidian
