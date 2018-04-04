#include "iccad2017SolutionQuality.h"

namespace ophidian
{
namespace legalization
{

ICCAD2017SolutionQuality::ICCAD2017SolutionQuality(ophidian::design::Design &design, ophidian::design::Design &originalDesign):
    mInitialLocations(design.netlist().makeProperty<util::Location>(ophidian::circuit::Cell())),
    mInitialFixed(design.netlist().makeProperty<bool>(ophidian::circuit::Cell())),
    mDesign(design),
    mOriginalDesign(originalDesign)
{
    for(auto cellIt = design.netlist().begin(ophidian::circuit::Cell()); cellIt != design.netlist().end(ophidian::circuit::Cell()); cellIt++)
    {
        mInitialFixed[*cellIt] = design.placement().isFixed(*cellIt);
        mInitialLocations[*cellIt] = design.placement().cellLocation(*cellIt);
    }
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

float ICCAD2017SolutionQuality::hpwlScore(){
    double originalHPWL = 0;

    for (auto it = mOriginalDesign.netlist().begin(circuit::Net()); it < mOriginalDesign.netlist().end(circuit::Net()); it++) {
        originalHPWL += hpwl(mOriginalDesign, *it);
    }

    double currentHPWL = 0;

    for (auto it = mDesign.netlist().begin(circuit::Net()); it < mDesign.netlist().end(circuit::Net()); it++) {
        currentHPWL += hpwl(mDesign, *it);
    }

    float score = std::max((currentHPWL - originalHPWL) / originalHPWL, 0.0) * (1 + std::max(0.0 /* beta * fof */, 0.2));

    return score;
}

double ICCAD2017SolutionQuality::totalDisplacement(){
    double displacement = 0.0;
    for(auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++)
        displacement+= cellDisplacement(mDesign.placement().cellLocation(*cellIt), mInitialLocations[*cellIt]);
    return displacement;
}

double ICCAD2017SolutionQuality::avgDisplacement(){
    double displacement = 0.0;
    unsigned int amount = 0;
    for(auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++){
        if(mInitialFixed[*cellIt] == false){
            displacement+= cellDisplacement(mDesign.placement().cellLocation(*cellIt), mInitialLocations[*cellIt]);
            amount++;
        }
    }
    return (displacement/mRowHeight)/amount;
}

int ICCAD2017SolutionQuality::maximumCellMovement(){
    double maximumCellMovement = 0;
    ophidian::circuit::Cell cellID;
    for(auto cellIt = mDesign.netlist().begin(ophidian::circuit::Cell()); cellIt != mDesign.netlist().end(ophidian::circuit::Cell()); cellIt++){
        if(cellDisplacement(mDesign.placement().cellLocation(*cellIt), mInitialLocations[*cellIt]) >= maximumCellMovement){
        maximumCellMovement = cellDisplacement(mDesign.placement().cellLocation(*cellIt), mInitialLocations[*cellIt]);
        cellID = *cellIt;
        }
    }
        std::cout<<"Cell Name:"<<mDesign.netlist().name(cellID)<<std::endl;
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

double ICCAD2017SolutionQuality::hpwl(ophidian::design::Design & design, const ophidian::circuit::Net & net){
    double minX = std::numeric_limits<double>::max(),
           minY = std::numeric_limits<double>::max(),
           maxX = std::numeric_limits<double>::lowest(),
           maxY = std::numeric_limits<double>::lowest();

    for(auto pin : design.netlist().pins(net)){
        util::Location pinLocation;

        auto input = design.netlist().input(pin);
        auto output = design.netlist().output(pin);

        if (input != circuit::Input()) {
            pinLocation = design.placement().inputPadLocation(input);
        } else if (output != circuit::Output()) {
            pinLocation = design.placement().outputPadLocation(output);
        } else {
            pinLocation = design.placementMapping().location(pin);
        }

        minX = std::min(pinLocation.toPoint().x(), minX);
        minY = std::min(pinLocation.toPoint().y(), minY);
        maxX = std::max(pinLocation.toPoint().x(), maxX);
        maxY = std::max(pinLocation.toPoint().y(), maxY);
    }
    return std::abs(maxX - minX) + std::abs(maxY - minY);
}


double ICCAD2017SolutionQuality::hpwl(const circuit::Net &net){
    return hpwl(mDesign, net);
}

} // namespace legalization
} // namespace ophidian
