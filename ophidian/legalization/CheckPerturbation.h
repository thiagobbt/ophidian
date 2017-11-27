#ifndef CHECKPERTURBATION_H
#define CHECKPERTURBATION_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/Subrows.h>

namespace ophidian {
namespace legalization {
class CheckPerturbation
{
public:
    CheckPerturbation(design::Design & design, Subrows & subrows, std::vector<circuit::Cell> & cells);

    int numberOfPerturbedLines(Subrow subrow);

private:
    design::Design & mDesign;

    Subrows & mSubrows;

    entity_system::Property<Subrow, std::vector<circuit::Cell>> mSubrowsMultirowCells;
};
}
}

#endif // CHECKPERTURBATION_H
