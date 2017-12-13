#include "KDtreeLegalization.h"

namespace ophidian {
namespace legalization {

KDtreeLegalization::KDtreeLegalization(design::Design &design) : mDesign(design){

}

void KDtreeLegalization::build(){
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
        mKDTree.add(mDesign.placement().cellLocation(*cellIt).toPoint(), std::make_shared<circuit::Cell>(*cellIt));
    mKDTree.build(geometry::Box(mDesign.floorplan().chipOrigin().toPoint(), mDesign.floorplan().chipUpperRightCorner().toPoint()));
}

void KDtreeLegalization::splitTree(unsigned int k){
    mAncients = mKDTree.ancientNodes(k);
    mSubTrees = mKDTree.subTrees(k);
}

const std::vector<std::shared_ptr<ophidian::circuit::Cell>> KDtreeLegalization::ancients() const{
    return mAncients;
}

const std::vector<std::pair<std::vector<std::shared_ptr<ophidian::circuit::Cell>>, geometry::Box>> KDtreeLegalization::subTrees() const{
    return mSubTrees;
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

}
}
