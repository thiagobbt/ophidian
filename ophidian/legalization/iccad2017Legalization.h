#ifndef ICCAD2017LEGALIZATION_H
#define ICCAD2017LEGALIZATION_H

#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/design/Design.h>
#include <ophidian/legalization/RtreeLegalization.h>

namespace ophidian
{
namespace legalization
{
class iccad2017Legalization
{
public:
    iccad2017Legalization(ophidian::design::Design & design);

    void legalize();
private:
    void initializeTemporaryBlocs();
    void eraseTemporaryBlocs();
    void fixFencesCells(bool fix);
    void releaseFencesCells();
    void flipCells();
private:
    bool cellIsLegalized(circuit::Cell cell);

    ophidian::design::Design & mDesign;
    MultirowAbacus mMultirowAbacus;

    RtreeLegalization mRtreeLegalization;

    std::vector<circuit::Cell> mTemporaryBlocs;

    entity_system::Property<circuit::Cell, util::Location> mCellsInitialLocations;
};

} // namespace legalization
} // namespace ophidian
#endif // ICCAD2017LEGALIZATION_H
