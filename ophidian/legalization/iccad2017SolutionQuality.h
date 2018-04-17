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
        // score = (1 + St) * (1 + Shpwl + Sv) * Sam * Smm

        return /*(1 + runtimeScore()) **/ (1 + hpwlScore() + softConstraintScore()) * avgMovementScore() * maxMovementScore();
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

    struct placementConstraints {
        int maximumMovement;
        int maximumUtilization;
    };

    const std::unordered_map<std::string, placementConstraints> mConstraints = {
        {"des_perf_1",         {5,   100}},
        {"des_perf_a_md1",     {30,   75}},
        {"des_perf_a_md2",     {35,   70}},
        {"des_perf_b_md1",     {300,  67}},
        {"des_perf_b_md2",     {300,  95}},
        {"edit_dist_1_md1",    {15,  100}},
        {"edit_dist_a_md2",    {400, 100}},
        {"edit_dist_a_md3",    {32,   80}},
        {"fft_2_md2",          {20,  100}},
        {"fft_a_md2",          {400, 100}},
        {"fft_a_md3",          {20,   62}},
        {"pci_bridge32_a_md1", {200,  80}},
        {"pci_bridge32_a_md2", {25,  100}},
        {"pci_bridge32_b_md1", {400,  70}},
        {"pci_bridge32_b_md2", {35,   50}},
        {"pci_bridge32_b_md3", {35,   65}}
    };

    const double BETA = 1.0;
};



} // namespace legalization
} // namespace ophidian

#endif // ICCAD2017SOLUTIONQUALITY_H
