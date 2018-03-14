#ifndef CELLALIGNMENT_H
#define CELLALIGNMENT_H

#include <ophidian/design/Design.h>

namespace ophidian {
namespace legalization {
class CellAlignment
{
public:
    CellAlignment(design::Design & design);

    void alignCellsToSitesAndRows();

private:
    design::Design & mDesign;
};
}
}


#endif // CELLALIGNMENT_H
