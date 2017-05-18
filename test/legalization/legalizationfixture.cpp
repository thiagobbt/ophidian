#include "legalizationfixture.h"


CircuitFixture::CircuitFixture()
    : libraryMapping_(netlist_), placement_(netlist_),
      placementLibrary_(stdCells_), placementMapping_(placement_, placementLibrary_, netlist_, libraryMapping_)
{

}

ophidian::circuit::Cell CircuitFixture::addCell(ophidian::standard_cell::Cell stdCell, ophidian::util::Location cellLocation, unsigned numberOfPins, bool fixed)
{
    auto cell = netlist_.add(ophidian::circuit::Cell());
    for (unsigned pinIndex = 0; pinIndex < numberOfPins; ++pinIndex) {
        auto cellPin = netlist_.add(ophidian::circuit::Pin());
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
    floorplan_.chipUpperRightCorner(ophidian::util::Location(5, 4));
    auto site = floorplan_.add(ophidian::floorplan::Site(), "site", ophidian::util::Location(0.1, 1.0));
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 1.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 2.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 3.0), 50, site);

    auto cellStdCell = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 1))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    placementLibrary_.geometry(cellStdCell, stdCellGeometry);

    auto cellStdCellMultirow = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 2))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    placementLibrary_.geometry(cellStdCellMultirow, stdCellGeometryMR);

    auto cell1Location = ophidian::util::Location(1.0, 2.8);
    addCell(cellStdCell, cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(1.5, 2.8);
    addCell(cellStdCell, cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(2.5, 2.2);
    addCell(cellStdCell, cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(4.0, 3.0);
    addCell(cellStdCell, cell4Location, 2, false);

    auto cell5Location = ophidian::util::Location(3.0, 0.0);
    addCell(cellStdCell, cell5Location, 2, false);

    auto cell6Location = ophidian::util::Location(3.0, 1.0);
    addCell(cellStdCellMultirow, cell6Location, 2, true);

    auto cell7Location = ophidian::util::Location(3.5, 1.0);
    addCell(cellStdCell, cell7Location, 2, false);
}

MultirowAbacusFixture::MultirowAbacusFixture()
{
    floorplan_.chipOrigin(ophidian::util::Location(0, 0));
    floorplan_.chipUpperRightCorner(ophidian::util::Location(5, 4));
    auto site = floorplan_.add(ophidian::floorplan::Site(), "site", ophidian::util::Location(0.1, 1.0));
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 1.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 2.0), 50, site);
    floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 3.0), 50, site);

    auto cellStdCell = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 1))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    placementLibrary_.geometry(cellStdCell, stdCellGeometry);

    auto cellStdCellMultirow = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 2))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    placementLibrary_.geometry(cellStdCellMultirow, stdCellGeometryMR);

    auto cell1Location = ophidian::util::Location(1.0, 2.8);
    addCell(cellStdCell, cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(1.5, 2.8);
    addCell(cellStdCell, cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(2.5, 2.2);
    addCell(cellStdCell, cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(4.0, 3.0);
    addCell(cellStdCell, cell4Location, 2, false);

    auto cell5Location = ophidian::util::Location(3.0, 0.0);
    addCell(cellStdCell, cell5Location, 2, false);

    auto cell6Location = ophidian::util::Location(3.0, 1.0);
    addCell(cellStdCellMultirow, cell6Location, 2, false);

    auto cell7Location = ophidian::util::Location(3.5, 1.0);
    addCell(cellStdCell, cell7Location, 2, false);

    auto cell8Location = ophidian::util::Location(3.5, 2.0);
    addCell(cellStdCell, cell8Location, 2, false);
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
    addCell(cellStdCell_, cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(2.0, 0.0);
    addCell(cellStdCell_, cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(3.0, 3.0);
    addCell(cellStdCell_, cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(4.0, 1.0);
    addCell(cellStdCell_, cell4Location, 2, false);
}

