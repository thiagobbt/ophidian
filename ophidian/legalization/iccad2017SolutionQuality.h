#ifndef ICCAD2017SOLUTIONQUALITY_H
#define ICCAD2017SOLUTIONQUALITY_H

#include <ophidian/design/Design.h>

namespace ophidian
{
namespace legalization
{
class ICCAD2017SolutionQuality{
public:
    ICCAD2017SolutionQuality(ophidian::design::Design &design);

    float rawScore(){
        return avgMovementScore() * maxMovementScore() * (1 +/* hpwlScore()*/ + softConstraintScore()) * (1 + runtimeScore());
    }

    double totalDisplacement();

    int maximumCellMovement();

private:
    float avgMovementScore();
    float maxMovementScore();
    /*float hpwlScore();*/
    float softConstraintScore(){return 0.2;}
    float runtimeScore(){return -0.2;}

    double cellDisplacement(util::Location l1, util::Location l2){
        return std::abs(l1.toPoint().y() - l2.toPoint().y()) + std::abs(l1.toPoint().x() - l2.toPoint().x());
    }

    float fmm();
    double hpwl(const ophidian::circuit::Net & net);

    ophidian::entity_system::Property<ophidian::circuit::Cell, util::Location> mInitialLocations;
    ophidian::design::Design & mDesign;
    double mRowHeight;
};



} // namespace legalization
} // namespace ophidian

#endif // ICCAD2017SOLUTIONQUALITY_H
