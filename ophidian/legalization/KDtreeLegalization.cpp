#include "KDtreeLegalization.h"

namespace ophidian {
namespace legalization {

KDtreeLegalization::KDtreeLegalization(design::Design &design) : mDesign(design), mCellsInitialLocations(design.netlist().makeProperty<util::Location>(circuit::Cell())){
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
        mCellsInitialLocations[*cellIt] = mDesign.placement().cellLocation(*cellIt);
    }
}

void KDtreeLegalization::build(ophidian::geometry::Box legalizationArea, unsigned int i){
    mPlaceableArea = util::MultiBox({legalizationArea});
    removeMacroblocksOverlaps();//move cells outside macroblocks
    allignCellsToNearestSite();//TODO: make sure to insert all cells inside chip boundaries

    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
        mKDTree.add(mDesign.placement().cellLocation(*cellIt).toPoint(), std::make_shared<circuit::Cell>(*cellIt));


    mKDTree.build(legalizationArea);

    mAncients = mKDTree.ancientNodes(i);
    mPartitions = mKDTree.partitions(i);

    for (auto cell_it = mDesign.netlist().begin(ophidian::circuit::Cell()); cell_it != mDesign.netlist().end(ophidian::circuit::Cell()); cell_it++)
        mDesign.placement().placeCell(*cell_it, mCellsInitialLocations[*cell_it]);
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
        std::vector<std::shared_ptr<LegalizationKDtree::Partition>> unlegalizedPartitions;
        for(auto & partition : mPartitions)
            if(partition.legalized == false){
                legalized = false;
                unlegalizedPartitions.push_back(std::make_shared<LegalizationKDtree::Partition>(partition));
            }

        //get unlegalized parent's partition
        if(legalized == false)
        {
            //check if node is root of KDtree.
            if(unlegalizedPartitions.at(0)->root->parent.get() == NULL)
            {
                legalized = true;
                std::cout<<"WARNING: Circuit is ILEGAL, the legalization could not be possible even for the whole circuit partition."<<std::endl;
            }else{
                mPartitions.clear();
                mPartitions = mKDTree.parentPartitions(unlegalizedPartitions);
                for(auto & partition : mPartitions)
                    for(auto cell : partition.elements)
                        if(!mDesign.placement().isFixed(*cell))
                            mDesign.placement().placeCell(*cell, mCellsInitialLocations[*cell]);
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

void KDtreeLegalization::removeMacroblocksOverlaps(){
    std::vector<circuit::Cell> cells(mDesign.netlist().begin(circuit::Cell()), mDesign.netlist().end(circuit::Cell()));

    ophidian::legalization::Subrows subrows(mDesign.netlist(), mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping());
    subrows.createSubrows(cells, mPlaceableArea);
    rtree macroblocks_boxes_rtree;
    //create macroblocks rtree
    for (auto cell_it = mDesign.netlist().begin(circuit::Cell()); cell_it != mDesign.netlist().end(circuit::Cell()); ++cell_it)
    {
        if(mDesign.placement().isFixed(*cell_it))
        {
            auto cellGeometry = mDesign.placementMapping().geometry(*cell_it);
            for(auto cell_box : cellGeometry)
            {
                macroblocks_boxes_rtree.insert(std::make_pair(cell_box, *cell_it));
            }
        }
    }

    //check if cell overlap a macroblock
    for (auto cell_it = mDesign.netlist().begin(circuit::Cell()); cell_it != mDesign.netlist().end(circuit::Cell()); ++cell_it)
    {
        if(!mDesign.placement().isFixed(*cell_it))
        {
            auto cellGeometry = mDesign.placementMapping().geometry(*cell_it);
            for(auto cell_box : cellGeometry)
            {
                std::vector<rtree_node> intersecting_nodes;
                macroblocks_boxes_rtree.query( boost::geometry::index::contains(cell_box), std::back_inserter(intersecting_nodes));
                macroblocks_boxes_rtree.query( boost::geometry::index::overlaps(cell_box), std::back_inserter(intersecting_nodes));
                if (!intersecting_nodes.empty())
                {
                    std::vector<ophidian::legalization::Subrow> closestSubrow;
                    auto cellLocation = mDesign.placement().cellLocation(*cell_it);
                    subrows.findClosestSubrows(1, cellLocation, closestSubrow);
                    auto origin = subrows.origin(closestSubrow.at(0));
                    auto upperRightCorner = subrows.upperCorner(closestSubrow.at(0));
                    if(cellLocation.toPoint().x() <= origin.toPoint().x())
                        mDesign.placement().placeCell(*cell_it, ophidian::util::Location(origin.toPoint().x(), origin.toPoint().y()));
                    else if(cell_box.max_corner().x() >= upperRightCorner.toPoint().x())
                        mDesign.placement().placeCell(*cell_it, ophidian::util::Location(upperRightCorner.toPoint().x() - (cell_box.max_corner().x() - cell_box.min_corner().x()), origin.toPoint().y()));
                    else
                        mDesign.placement().placeCell(*cell_it, ophidian::util::Location(cellLocation.toPoint().x(), origin.toPoint().y()));
                }
            }
        }
    }
}

}
}
