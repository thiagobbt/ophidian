#include "KDtreeLegalization.h"

namespace ophidian {
namespace legalization {

KDtreeLegalization::KDtreeLegalization(design::Design &design) : mDesign(design){

}

void KDtreeLegalization::build(){
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
        mKDTree.add(mDesign.placement().cellLocation(*cellIt).toPoint(), std::make_shared<circuit::Cell>(*cellIt));

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
    mKDTree.build(geometry::Box(origin, upperCorner));
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
