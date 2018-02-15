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

    mAncients = mKDTree.ancientNodes(4);
    mPartitions = mKDTree.partitions(4);
}

void KDtreeLegalization::legalize(){
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

    bool legalized = false;
    while(legalized == false)
    {
        //PARALEL
        #pragma omp parallel for
        for(auto partition_it = mPartitions.begin(); partition_it < mPartitions.end(); partition_it++)
        {
            std::vector<ophidian::circuit::Cell> cellsToLegalize;
            for(auto ancient : ancientsAndFixeds)
                cellsToLegalize.push_back(ancient);
            for(auto cell : partition_it->elements)
                cellsToLegalize.push_back(*cell);

            //legalize partition
            MultirowAbacus multirowAbacus(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
            partition_it->legalized = multirowAbacus.legalizePlacement(cellsToLegalize, util::MultiBox({partition_it->range}));
        }

        //SEQUENTIAL
//        for(auto & partition : mPartitions)
//        {
//            std::vector<ophidian::circuit::Cell> cellsToLegalize;
//            for(auto ancient : ancientsAndFixeds)
//                cellsToLegalize.push_back(ancient);
//            for(auto cell : partition.elements)
//                cellsToLegalize.push_back(*cell);

//            //legalize partition
//            MultirowAbacus multirowAbacus(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
//            partition.legalized = multirowAbacus.legalizePlacement(cellsToLegalize, util::MultiBox({partition.range}));
//        }

        //get unlegalized partitions
        legalized = true;
        std::vector<std::shared_ptr<LegalizationKDtree::Node>> unlegalizedPartitions;
        for(auto & partition : mPartitions)
            if(partition.legalized == false){
                legalized = false;
                unlegalizedPartitions.push_back(partition.root->parent);
            }

        //get unlegalized parent's partition
        if(legalized == false)
        {
            //check if node is root of KDtree.
            if(unlegalizedPartitions.at(0)->parent.get() == NULL)
            {
                legalized = true;
                std::cout<<"WARNING: Circuit is ILEGAL, the legalization could not be possible even for the whole circuit partition."<<std::endl;
            }else{
                mPartitions.clear();
                mPartitions = mKDTree.parentPartitions(unlegalizedPartitions);
            }
        }
    }
}

void KDtreeLegalization::density() const{
    for(auto partition : mPartitions){
        double total_cell_area = 0.0;
        unsigned int cells =0;
        for(auto ancient : mAncients){
            auto ancient_location = mDesign.placement().cellLocation(*ancient);
            if(boost::geometry::within(ancient_location.toPoint(), partition.range)){
                total_cell_area+= boost::geometry::area(mDesign.placementMapping().geometry(*ancient)[0]);
                cells++;
            }
        }
        for(auto partition_cell : partition.elements)
            total_cell_area+= boost::geometry::area(mDesign.placementMapping().geometry(*partition_cell)[0]);
        std::cout<<"Number of cells: "<<(cells+partition.elements.size())<<" Density: "<<total_cell_area/boost::geometry::area(partition.range)<<std::endl;
    }
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
