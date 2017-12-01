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
        std::cout << "fence index " << fenceIndex++ << std::endl;
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
    //posiciona fences (paralelo)
    legalizeFences();

    mFenceRegionIsolation.isolateAllFenceCells();

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


    mFenceRegionIsolation.restoreAllFenceCells();

    //flip cells
    flipCells();
}

void iccad2017Legalization::allignCellsToNearestSite(){
    for(auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++){
        if(!mDesign.placement().isFixed(*cellIt)){
            auto initialCellLocation = mDesign.placement().cellLocation(*cellIt);
            auto cellAlignment = mDesign.placementMapping().alignment(*cellIt);

            auto siteHeight = units::unit_cast<double>(mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).y());
            auto siteWidth = units::unit_cast<double>(mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).x());

            double nearSiteX = std::round(initialCellLocation.toPoint().x() / siteWidth) * siteWidth;
            double nearSiteY = std::round(initialCellLocation.toPoint().y() / siteHeight) * siteHeight;


            placement::RowAlignment rowAligment = (((int)(nearSiteY / siteHeight) % 2) == 0) ? placement::RowAlignment::EVEN : placement::RowAlignment::ODD;
            if(cellAlignment == rowAligment)
                mDesign.placement().placeCell(*cellIt, util::Location(nearSiteX, nearSiteY));
            else if(initialCellLocation.toPoint().y() >= nearSiteY+0.5*siteHeight)
                mDesign.placement().placeCell(*cellIt, util::Location(nearSiteX, nearSiteY+siteHeight));
            else
                mDesign.placement().placeCell(*cellIt, util::Location(nearSiteX, nearSiteY-siteHeight));
        }
    }

}

void iccad2017Legalization::ancientLegalization()
{
    legalizeFences();

    mFenceRegionIsolation.isolateAllFenceCells();

    //make sure to insert all cells inside chip boundaries

    //alling all cells to nearest site in order to avoid dead rows/cells
    allignCellsToNearestSite();

    //initialize and split kdtree
    KDtreeLegalization kdtree(mDesign);
    kdtree.build();
    unsigned int i = 4;
    kdtree.splitTree(i);

//    kdtree.density();

    //legalize ancients and fix then
    std::vector<ophidian::circuit::Cell> ancients;
    for(auto cellIt : kdtree.ancients())
        ancients.push_back(*cellIt);

    for (auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++)
    {
        if (mDesign.placement().isFixed(*cellIt))
            ancients.push_back(*cellIt);
    }

    MultirowAbacus multirowAbacus1(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
    multirowAbacus1.legalizePlacement(ancients, util::MultiBox({geometry::Box(mDesign.floorplan().chipOrigin().toPoint(), mDesign.floorplan().chipUpperRightCorner().toPoint())}));



    for(auto subTree : kdtree.subTrees())
    {
        //get ancients and fixed cells
        std::vector<ophidian::circuit::Cell> cellsToLegalize;
        for(auto ancient : ancients)
            cellsToLegalize.push_back(ancient);
        for(auto cell : subTree.first)
            cellsToLegalize.push_back(*cell);

        //legalize sub tree
        MultirowAbacus multirowAbacus(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
        multirowAbacus.legalizePlacement(cellsToLegalize, util::MultiBox({subTree.second}));
    }

    mFenceRegionIsolation.restoreAllFenceCells();

    flipCells();
}

} // namespace legalization
} // namespace ophidian
