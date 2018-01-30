#ifndef RECTILINEARFENCEFIXTURE_H
#define RECTILINEARFENCEFIXTURE_H

#include "legalizationfixture.h"


class RectilinearFenceFixture : public LargerLegalCircuitFixture
{
public:
    RectilinearFenceFixture();

protected:
    ophidian::placement::Fence mFenceRegion;
};

#endif // RECTILINEARFENCEFIXTURE_H
