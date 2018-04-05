#ifndef ICCAD2017SOLUTIONQUALITY_H
#define ICCAD2017SOLUTIONQUALITY_H

#include <ophidian/design/Design.h>

#define DEBUG(x) [&]() { std::cerr << #x << ":" << x << std::endl; }()

namespace ophidian
{
namespace legalization
{
class ICCAD2017SolutionQuality{
public:
    ICCAD2017SolutionQuality(ophidian::design::Design &design, ophidian::design::Design &originalDesign);

    double avgMovementScore();
    double maxMovementScore();
    double hpwlScore();
    double softConstraintScore() { return 0.2; }
    double runtimeScore() { return -0.2; }
    double rawScore(){
        DEBUG(avgMovementScore());
        DEBUG(maxMovementScore());
        DEBUG(hpwlScore());
        DEBUG(softConstraintScore());
        DEBUG(runtimeScore());
        return avgMovementScore() * maxMovementScore() * (1 + hpwlScore() + softConstraintScore()) * (1 + runtimeScore());
    }

    double totalDisplacement();

    double avgDisplacement();

    int maximumCellMovement();

private:
    double cellDisplacement(util::Location l1, util::Location l2){
        return std::abs(l1.toPoint().y() - l2.toPoint().y()) + std::abs(l1.toPoint().x() - l2.toPoint().x());
    }

    double fmm();
    double fof();
    double hpwl(const ophidian::circuit::Net & net);
    double hpwl(ophidian::design::Design & design, const ophidian::circuit::Net & net);

    ophidian::entity_system::Property<ophidian::circuit::Cell, util::Location> mInitialLocations;
    ophidian::entity_system::Property<ophidian::circuit::Cell, bool> mInitialFixed;
    ophidian::design::Design & mDesign;
    ophidian::design::Design & mOriginalDesign;
    const double mRowHeight;

    const std::unordered_map<std::string, unsigned int> mMaximumMovement = {
        {"des_perf_b_md1", 300},
        {"des_perf_b_md2", 300},
        {"edit_dist_1_md1", 15},
        {"edit_dist_a_md2", 400},
        {"fft_2_md2", 20},
        {"fft_a_md2", 400},
        {"fft_a_md3", 20},
        {"pci_bridge32_a_md1", 200}
    };

    const double BETA = 1.0;
};



} // namespace legalization
} // namespace ophidian

#endif // ICCAD2017SOLUTIONQUALITY_H
