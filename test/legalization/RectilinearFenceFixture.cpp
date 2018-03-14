#include "RectilinearFenceFixture.h"

RectilinearFenceFixture::RectilinearFenceFixture()
 : LargerLegalCircuitFixture() {
    mFenceRegion = design_.fences().add("fence0");

    ophidian::geometry::Box box1(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20));
    ophidian::geometry::Box box2(ophidian::geometry::Point(10, 0), ophidian::geometry::Point(20, 10));
    design_.fences().area(mFenceRegion, ophidian::util::MultiBox({box1, box2}));
}
