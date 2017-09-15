#include "iccad2017SolutionQuality.h"

namespace ophidian
{
namespace legalization
{

ICCAD2017SolutionQuality::ICCAD2017SolutionQuality(ophidian::design::Design &design):mDesign(design), mInitialLocations(design.netlist().makeProperty<util::Location>(ophidian::circuit::Cell())){
    for(auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); cellIt++)
        mInitialLocations[*cellIt] = design.placement().cellLocation(*cellIt);
    mRowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();
}

float ICCAD2017SolutionQuality::avgMovementScore(){
    std::unordered_map<double, std::pair<double, std::size_t>> cellHeight2Counter;
    for(auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++)
        if(!mDesign.placement().isFixed(*cellIt))
        {
            auto cellHeight = mDesign.placementMapping().geometry(*cellIt)[0].max_corner().y() - mDesign.placementMapping().geometry(*cellIt)[0].min_corner().y();
            double displacement = cellDisplacement(mDesign.placement().cellLocation(*cellIt), mInitialLocations[*cellIt]);
            auto displacementHeightCounter = cellHeight2Counter.find(cellHeight);
            if(displacementHeightCounter == cellHeight2Counter.end())
                cellHeight2Counter[cellHeight] = std::pair<double, std::size_t>(displacement, 1);
            else
                cellHeight2Counter[cellHeight] = std::pair<double, size_t>(cellHeight2Counter[cellHeight].first + displacement, cellHeight2Counter[cellHeight].second + 1);
        }
    float averageDisplacement = 0;
    for(auto displacementCounter : cellHeight2Counter)
        averageDisplacement += (displacementCounter.second.first/displacementCounter.second.second);
    averageDisplacement = (averageDisplacement / mRowHeight) / cellHeight2Counter.size();
    return averageDisplacement;
}

float ICCAD2017SolutionQuality::maxMovementScore(){
    return 1 + (maximumCellMovement()/100.0) * fmm();
}

int ICCAD2017SolutionQuality::maximumCellMovement(){
    double maximumCellMovement = 0;
    for(auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++)
        maximumCellMovement = std::max(maximumCellMovement, cellDisplacement(mDesign.placement().cellLocation(*cellIt), mInitialLocations[*cellIt]));
    return maximumCellMovement / mRowHeight;
}

float ICCAD2017SolutionQuality::fmm(){
    std::unordered_map<std::string, unsigned int> maximumMovement = {{"des_perf_b_md1", 300}, {"des_perf_b_md2", 300}, {"edit_dist_1_md1", 15}, {"edit_dist_a_md2", 400}, {"fft_2_md2", 20}, {"fft_a_md2", 400}, {"fft_a_md3", 20}, {"pci_bridge32_a_md1", 200}};
    std::string circuitName = mDesign.circuitName();
    double displacement = 0;
    for(auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++)
        if(cellDisplacement(mDesign.placement().cellLocation(*cellIt), mInitialLocations[*cellIt])/mRowHeight > maximumMovement[circuitName])
            displacement += cellDisplacement(mDesign.placement().cellLocation(*cellIt), mInitialLocations[*cellIt]);
    displacement = (displacement / mRowHeight) / maximumMovement[circuitName];
    return std::max(displacement, 1.0);
}

double ICCAD2017SolutionQuality::hpwl(const circuit::Net &net){
    double minX = std::numeric_limits<double>::max(), minY = std::numeric_limits<double>::max(),
           maxX = std::numeric_limits<double>::lowest(), maxY = std::numeric_limits<double>::lowest();
    for(auto pin : mDesign.netlist().pins(net)){
        minX = std::min(mDesign.placementMapping().location(pin).toPoint().x(), minX);
        minY = std::min(mDesign.placementMapping().location(pin).toPoint().y(), minY);
        maxX = std::max(mDesign.placementMapping().location(pin).toPoint().x(), maxX);
        maxY = std::max(mDesign.placementMapping().location(pin).toPoint().y(), maxY);
    }
    return std::abs(maxX - minX) + std::abs(maxY - minY);
}

} // namespace legalization
} // namespace ophidian
