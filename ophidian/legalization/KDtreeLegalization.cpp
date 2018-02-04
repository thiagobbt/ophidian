#include "KDtreeLegalization.h"

namespace ophidian {
namespace legalization {

KDtreeLegalization::KDtreeLegalization(design::Design &design) : mDesign(design){

}

void KDtreeLegalization::build(ophidian::geometry::Box legalizationArea){
    allignCellsToNearestSite();//TODO: make sure to insert all cells inside chip boundaries

    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
        mKDTree.add(mDesign.placement().cellLocation(*cellIt).toPoint(), std::make_shared<circuit::Cell>(*cellIt));

    mPlaceableArea = util::MultiBox({legalizationArea});
    mKDTree.build(legalizationArea);
}

void KDtreeLegalization::legalize(){
    splitTree(4);

    //legalize ancients and fix then
    std::vector<ophidian::circuit::Cell> ancientsAndFixeds;
    for(auto cellIt : mAncients)
        ancientsAndFixeds.push_back(*cellIt);

    for (auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++)
        if (mDesign.placement().isFixed(*cellIt))
            ancientsAndFixeds.push_back(*cellIt);

    MultirowAbacus multirowAbacus1(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
    multirowAbacus1.legalizePlacement(ancientsAndFixeds, mPlaceableArea);

    for(auto cellIt : mAncients)
        mDesign.placement().fixLocation(*cellIt, true);

    //PARALEL
#pragma omp parallel for
    for(auto subTree_it = mSubTrees.begin(); subTree_it < mSubTrees.end(); subTree_it++)
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
//    for(auto subTree : mSubTrees)
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
}

void KDtreeLegalization::density() const{
    for(auto partition : mSubTrees){
        double total_cell_area = 0.0;
        unsigned int cells =0;
        for(auto ancient : mAncients){
            auto ancient_location = mDesign.placement().cellLocation(*ancient);
            if(boost::geometry::within(ancient_location.toPoint(), partition.second)){
                total_cell_area+= boost::geometry::area(mDesign.placementMapping().geometry(*ancient)[0]);
                cells++;
            }
        }
        for(auto partition_cell : partition.first)
            total_cell_area+= boost::geometry::area(mDesign.placementMapping().geometry(*partition_cell)[0]);
        std::cout<<"Number of cells: "<<(cells+partition.first.size())<<" Density: "<<total_cell_area/boost::geometry::area(partition.second)<<std::endl;
    }
}

void KDtreeLegalization::splitTree(unsigned int k){
    mAncients = mKDTree.ancientNodes(k);
    mSubTrees = mKDTree.subTrees(k);
}

void KDtreeLegalization::allignCellsToNearestSite(){
    for(auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++)
    {
        if(!mDesign.placement().isFixed(*cellIt))
        {
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

}
}
