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
            if(cellAlignment == rowAligment || cellAlignment == placement::RowAlignment::NA)
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

    //alling all cells to nearest site in order to avoid dead rows/collumns
    allignCellsToNearestSite();

    //initialize and split kdtree
    KDtreeLegalization kdtree(mDesign);
    kdtree.build();
    unsigned int i = 4;
    kdtree.splitTree(i);

//    kdtree.density();

    //legalize ancients and fix then
    std::vector<ophidian::circuit::Cell> ancientsAndFixeds;
    for(auto cellIt : kdtree.ancients()){
        ancientsAndFixeds.push_back(*cellIt);
//        std::cout<<mDesign.netlist().name(*cellIt)<<" ("<<mDesign.placement().cellLocation(*cellIt).x()<<", "<<mDesign.placement().cellLocation(*cellIt).y()<<")"<<" is fixed: "<<mDesign.placement().isFixed(*cellIt)<<std::endl;
    }

    for (auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++)
    {
        if (mDesign.placement().isFixed(*cellIt))
            ancientsAndFixeds.push_back(*cellIt);
    }

    MultirowAbacus multirowAbacus1(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
    double minX = std::numeric_limits<double>::max(), minY = std::numeric_limits<double>::max(), maxX = std::numeric_limits<double>::min(), maxY = std::numeric_limits<double>::min();
    for (auto row : mDesign.floorplan().rowsRange()) {
        auto rowOrigin = mDesign.floorplan().origin(row).toPoint();
        auto rowUpperCorner = geometry::Point(rowOrigin.x() + mDesign.floorplan().rowUpperRightCorner(row).toPoint().x(), rowOrigin.y() + mDesign.floorplan().rowUpperRightCorner(row).toPoint().y());
        minX = std::min(minX, std::min(rowOrigin.x(), rowUpperCorner.x()));
        minY = std::min(minY, std::min(rowOrigin.y(), rowUpperCorner.y()));
        maxX = std::max(maxX, std::max(rowOrigin.x(), rowUpperCorner.x()));
        maxY = std::max(maxY, std::max(rowOrigin.y(), rowUpperCorner.y()));
    }
    geometry::Point origin(minX, minY);
    geometry::Point upperCorner(maxX, maxY);
//    std::cout<<"Legalization area: ("<<origin.x()<<", "<<origin.y()<<") ("<<upperCorner.x()<<", "<<upperCorner.y()<<")"<<std::endl;
    multirowAbacus1.legalizePlacement(ancientsAndFixeds, util::MultiBox({geometry::Box(origin, upperCorner)}));

    for(auto cellIt : kdtree.ancients()){
        mDesign.placement().fixLocation(*cellIt, true);
//        std::cout<<mDesign.netlist().name(*cellIt)<<" ("<<mDesign.placement().cellLocation(*cellIt).x()<<", "<<mDesign.placement().cellLocation(*cellIt).y()<<")"<<" is fixed: "<<mDesign.placement().isFixed(*cellIt)<<std::endl;
    }

    //PARALEL
    auto tree = kdtree.subTrees();
#pragma omp parallel for
    for(auto subTree_it = tree.begin(); subTree_it < tree.end(); subTree_it++)
    {
        std::vector<ophidian::circuit::Cell> cellsToLegalize;
        for(auto ancient : ancientsAndFixeds)
            cellsToLegalize.push_back(ancient);
        for(auto cell : subTree_it->first)
            cellsToLegalize.push_back(*cell);

        //legalize sub tree
        MultirowAbacus multirowAbacus(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
        multirowAbacus.legalizePlacement(cellsToLegalize, util::MultiBox({subTree_it->second}));
    }

    //SEQUENTIAL
//    for(auto subTree : kdtree.subTrees())
//    {
//        std::vector<ophidian::circuit::Cell> cellsToLegalize;
//        for(auto ancient : ancientsAndFixeds)
//            cellsToLegalize.push_back(ancient);
//        for(auto cell : subTree.first)
//            cellsToLegalize.push_back(*cell);



//        //legalize sub tree
//        MultirowAbacus multirowAbacus(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
//        multirowAbacus.legalizePlacement(cellsToLegalize, util::MultiBox({subTree.second}));
//    }

    mFenceRegionIsolation.restoreAllFenceCells();

    flipCells();
}

} // namespace legalization
} // namespace ophidian
