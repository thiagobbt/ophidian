#ifndef ICCAD2017LEGALIZATION_H
#define ICCAD2017LEGALIZATION_H

#include <ophidian/legalization/MultirowAbacus.h>
#include <ophidian/design/Design.h>

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
private:
    ophidian::design::Design & mDesign;
    MultirowAbacus mMultirowAbacus;

    std::vector<circuit::Cell> mTemporaryBlocs;
};

} // namespace legalization
} // namespace ophidian
#endif // ICCAD2017LEGALIZATION_H
