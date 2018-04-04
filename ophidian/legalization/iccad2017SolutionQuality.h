#ifndef ICCAD2017SOLUTIONQUALITY_H
#define ICCAD2017SOLUTIONQUALITY_H

#include <ophidian/design/Design.h>

namespace ophidian
{
namespace legalization
{
class ICCAD2017SolutionQuality{
public:
    ICCAD2017SolutionQuality(ophidian::design::Design &design, ophidian::design::Design &originalDesign);

    float rawScore(){
        return avgMovementScore() * maxMovementScore() * (1 + hpwlScore() + softConstraintScore()) * (1 + runtimeScore());
    }

    double totalDisplacement();

    double avgDisplacement();

    int maximumCellMovement();

private:
    float avgMovementScore();
    float maxMovementScore();
    float hpwlScore();
    float softConstraintScore(){return 0.2;}
    float runtimeScore(){return -0.2;}

    double cellDisplacement(util::Location l1, util::Location l2){
        return std::abs(l1.toPoint().y() - l2.toPoint().y()) + std::abs(l1.toPoint().x() - l2.toPoint().x());
    }

    float fmm();
    double hpwl(const ophidian::circuit::Net & net);
    double hpwl(ophidian::design::Design & design, const ophidian::circuit::Net & net);

    ophidian::entity_system::Property<ophidian::circuit::Cell, util::Location> mInitialLocations;
    ophidian::entity_system::Property<ophidian::circuit::Cell, bool> mInitialFixed;
    ophidian::design::Design & mDesign;
    ophidian::design::Design & mOriginalDesign;
    double mRowHeight;
};



} // namespace legalization
} // namespace ophidian

#endif // ICCAD2017SOLUTIONQUALITY_H
