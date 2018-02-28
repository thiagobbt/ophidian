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
      mFenceRegionIsolation(design)
{
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
        mCellsInitialLocations[*cellIt] = mDesign.placement().cellLocation(*cellIt);
    }
}

void iccad2017Legalization::legalizeFences()
{
    std::size_t fenceIndex = 0;
    for(auto fenceIt = mDesign.fences().range().begin(); fenceIt < mDesign.fences().range().end(); fenceIt++)
    {
//        std::cout << "fence index " << fenceIndex++ << std::endl;
        auto fence = *fenceIt;
        std::vector<circuit::Cell> cells (mDesign.fences().members(fence).begin(), mDesign.fences().members(fence).end());
        mMultirowAbacus.legalizePlacement(cells, mDesign.fences().area(fence));
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

void iccad2017Legalization::legalize()
{
    //generate the placeable area
    isolateFloorplan();
    //it could be done in parallel
    legalizeFences();

    mFenceRegionIsolation.isolateAllFenceCells();

    //place circuit cells
    std::vector<circuit::Cell> cells;
    cells.reserve(mDesign.netlist().size(circuit::Cell()));
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
    {
        if(!mDesign.placement().cellHasFence(*cellIt))
        {
            cells.push_back(*cellIt);
        }
    }
    mMultirowAbacus.legalizePlacement(cells, util::MultiBox({mPlaceableArea}));

    restoreFloorplan();

    mFenceRegionIsolation.restoreAllFenceCells();

    //flip cells
    flipCells();
}

void iccad2017Legalization::kdtreeLegalization(unsigned int i)
{
    //generate the placeable area
    isolateFloorplan();
    //it could be done in parallel
    legalizeFences();

    mFenceRegionIsolation.isolateAllFenceCells();

    KDtreeLegalization kdtreeLegalization(mDesign);
    kdtreeLegalization.build(mPlaceableArea, i);
    kdtreeLegalization.legalize();

    restoreFloorplan();

    mFenceRegionIsolation.restoreAllFenceCells();

    flipCells();    
}

void iccad2017Legalization::isolateFloorplan(){
    util::MultiBox rows;
    for(auto rowIt = mDesign.floorplan().rowsRange().begin(); rowIt != mDesign.floorplan().rowsRange().end(); rowIt++){
        auto rowOrigin = mDesign.floorplan().origin(*rowIt);
        auto rowSize = mDesign.floorplan().rowUpperRightCorner(*rowIt);
        auto rowUpperRightCorner = ophidian::geometry::Point(rowOrigin.toPoint().x() + rowSize.toPoint().x(), rowOrigin.toPoint().y() + rowSize.toPoint().y());
        rows.push_back(ophidian::geometry::Box(rowOrigin.toPoint(), rowUpperRightCorner));
    }
    boost::geometry::envelope(rows.toMultiPolygon(), mPlaceableArea);

    ophidian::geometry::MultiPolygon difference;
    boost::geometry::difference(util::MultiBox({mPlaceableArea}).toMultiPolygon(), rows.toMultiPolygon(), difference);

    for(auto polygon : difference)
    {
        ophidian::geometry::Box macroBlockBox;
        boost::geometry::envelope(polygon, macroBlockBox);

        std::string cellName = "FloorplanBlock" + boost::lexical_cast<std::string>(mCountFloorplanBlocks);
        auto stdCell = mDesign.standardCells().add(ophidian::standard_cell::Cell(), cellName);
        auto circuitCell = mDesign.netlist().add(ophidian::circuit::Cell(), cellName);

        geometry::Box traslatedBox;
        geometry::translate(macroBlockBox, -macroBlockBox.min_corner().x(), -macroBlockBox.min_corner().y(), traslatedBox);

        mDesign.library().geometry(stdCell, util::MultiBox({traslatedBox}));
        mDesign.library().cellAlignment(stdCell, placement::RowAlignment::NA);
        mDesign.placement().placeCell(circuitCell, util::Location(macroBlockBox.min_corner().x(), macroBlockBox.min_corner().y()));
        mDesign.placement().fixLocation(circuitCell, true);
        mDesign.libraryMapping().cellStdCell(circuitCell, stdCell);

        mFloorplanBlocks.push_back(circuitCell);
        mCountFloorplanBlocks++;
    }
}

void iccad2017Legalization::restoreFloorplan(){
    for(auto block : mFloorplanBlocks)
        mDesign.netlist().erase(block);
}

} // namespace legalization
} // namespace ophidian
