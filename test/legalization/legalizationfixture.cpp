#include "legalizationfixture.h"


CircuitFixture::CircuitFixture()
    : libraryMapping_(netlist_), placement_(netlist_), fences_(netlist_),
      placementLibrary_(stdCells_), placementMapping_(placement_, placementLibrary_, netlist_, libraryMapping_)
{
}

ophidian::circuit::Cell CircuitFixture::addCell(ophidian::standard_cell::Cell stdCell, std::string cellName, ophidian::util::Location cellLocation, unsigned numberOfPins, bool fixed)
{
    auto cell = netlist_.add(ophidian::circuit::Cell(), cellName);
    for (unsigned pinIndex = 0; pinIndex < numberOfPins; ++pinIndex) {
        auto cellPin = netlist_.add(ophidian::circuit::Pin(), "pin:"+std::to_string(pinIndex));
        netlist_.add(cell, cellPin);
    }
    libraryMapping_.cellStdCell(cell, stdCell);
    placement_.placeCell(cell, cellLocation);
    placement_.fixLocation(cell, fixed);
    return cell;
}

AbacusFixture::AbacusFixture()
{
    floorplan_.chipOrigin(ophidian::util::Location(0, 0));
    floorplan_.chipUpperRightCorner(ophidian::util::Location(50, 40));
    auto site = floorplan_.add(ophidian::floorplan::Site(), "site", ophidian::util::Location(1.0, 10.0));
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 50, site);

    auto cellStdCell = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    placementLibrary_.geometry(cellStdCell, stdCellGeometry);

    auto cellStdCellMultirow = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    placementLibrary_.geometry(cellStdCellMultirow, stdCellGeometryMR);

    auto cell1Location = ophidian::util::Location(10, 28);
    addCell(cellStdCell, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(15, 28);
    addCell(cellStdCell, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(25, 22);
    addCell(cellStdCell, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(40, 30);
    addCell(cellStdCell, "cell4", cell4Location, 2, false);

    auto cell5Location = ophidian::util::Location(30, 00);
    addCell(cellStdCell, "cell5", cell5Location, 2, false);

    auto cell6Location = ophidian::util::Location(30, 10);
    addCell(cellStdCellMultirow, "cell6", cell6Location, 2, true);

    auto cell7Location = ophidian::util::Location(35, 10);
    addCell(cellStdCell, "cell7", cell7Location, 2, false);
}

MultirowAbacusFixture::MultirowAbacusFixture()
{
    floorplan_.chipOrigin(ophidian::util::Location(0, 0));
    floorplan_.chipUpperRightCorner(ophidian::util::Location(50, 40));
    auto site = floorplan_.add(ophidian::floorplan::Site(), "site", ophidian::util::Location(1.0, 10.0));
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 50, site);

    auto cellStdCell = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    placementLibrary_.geometry(cellStdCell, stdCellGeometry);

    auto cellStdCellMultirow = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    placementLibrary_.geometry(cellStdCellMultirow, stdCellGeometryMR);

    auto cell1Location = ophidian::util::Location(10, 28);
    addCell(cellStdCell, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(15, 28);
    addCell(cellStdCell, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(25, 22);
    addCell(cellStdCell, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(40, 30);
    addCell(cellStdCell, "cell4", cell4Location, 2, false);

    auto cell5Location = ophidian::util::Location(30, 00);
    addCell(cellStdCell, "cell5", cell5Location, 2, false);

    auto cell6Location = ophidian::util::Location(30, 10);
    addCell(cellStdCellMultirow, "cell6", cell6Location, 2, false);

    auto cell7Location = ophidian::util::Location(35, 10);
    addCell(cellStdCell, "cell7", cell7Location, 2, false);

    auto cell8Location = ophidian::util::Location(35, 20);
    addCell(cellStdCell, "cell8", cell8Location, 2, false);
}

LegalCircuitFixture::LegalCircuitFixture()
{
    floorplan_.chipOrigin(ophidian::util::Location(0, 0));
    floorplan_.chipUpperRightCorner(ophidian::util::Location(5, 4));
    auto site = floorplan_.add(ophidian::floorplan::Site(), "site", ophidian::util::Location(1.0, 1.0));
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 5, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 1.0), 5, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 2.0), 5, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 3.0), 5, site);

    cellStdCell_ = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 1))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    placementLibrary_.geometry(cellStdCell_, stdCellGeometry);

    auto cell1Location = ophidian::util::Location(1.0, 2.0);
    addCell(cellStdCell_, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(2.0, 0.0);
    addCell(cellStdCell_, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(3.0, 3.0);
    addCell(cellStdCell_, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(4.0, 1.0);
    addCell(cellStdCell_, "cell4", cell4Location, 2, false);

    auto fence1 = fences_.add("fence1");
    fences_.area(fence1, ophidian::util::MultiBox({ophidian::geometry::Box({0, 0}, {3, 3})}));

    auto cell1 = netlist_.find(ophidian::circuit::Cell(), "cell1");
    fences_.connect(fence1, cell1);
    placement_.cellFence(cell1, fence1);

    auto cell2 = netlist_.find(ophidian::circuit::Cell(), "cell2");
    fences_.connect(fence1, cell2);
    placement_.cellFence(cell2, fence1);
}

LargerLegalCircuitFixture::LargerLegalCircuitFixture()
{
    floorplan_.chipOrigin(ophidian::util::Location(0, 0));
    floorplan_.chipUpperRightCorner(ophidian::util::Location(50, 40));
    auto site = floorplan_.add(ophidian::floorplan::Site(), "site", ophidian::util::Location(10.0, 10.0));
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 5, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 5, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 5, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 5, site);

    cellStdCell_ = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    placementLibrary_.geometry(cellStdCell_, stdCellGeometry);

    auto cell1Location = ophidian::util::Location(10.0, 20.0);
    addCell(cellStdCell_, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(20.0, 0.0);
    addCell(cellStdCell_, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(30.0, 30.0);
    addCell(cellStdCell_, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(40.0, 10.0);
    addCell(cellStdCell_, "cell4", cell4Location, 2, false);
}
