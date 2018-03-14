#include "legalizationfixture.h"


CircuitFixture::CircuitFixture()
{
}

ophidian::circuit::Cell CircuitFixture::addCell(ophidian::standard_cell::Cell stdCell, std::string cellName, ophidian::util::Location cellLocation, unsigned numberOfPins, bool fixed)
{
    auto cell = design_.netlist().add(ophidian::circuit::Cell(), cellName);
    for (unsigned pinIndex = 0; pinIndex < numberOfPins; ++pinIndex) {
        auto cellPin = design_.netlist().add(ophidian::circuit::Pin(), "pin:"+std::to_string(pinIndex));
        design_.netlist().add(cell, cellPin);
    }
    design_.libraryMapping().cellStdCell(cell, stdCell);
    design_.placement().placeCell(cell, cellLocation);
    design_.placement().fixLocation(cell, fixed);
    return cell;
}

AbacusFixture::AbacusFixture()
{
    design_.floorplan().chipOrigin(ophidian::util::Location(0, 0));
    design_.floorplan().chipUpperRightCorner(ophidian::util::Location(50, 40));
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(1.0, 10.0));
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 50, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 50, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 50, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 50, site);

    auto cellStdCell = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(cellStdCell, stdCellGeometry);

    auto cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);

    auto cell1Location = ophidian::util::Location(10, 28);
    addCell(cellStdCell, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(15, 28);
    addCell(cellStdCell, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(25, 22);
    addCell(cellStdCell, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(40, 30);
    addCell(cellStdCell, "cell4", cell4Location, 2, false);

    auto cell5Location = ophidian::util::Location(30, 0);
    addCell(cellStdCell, "cell5", cell5Location, 2, false);

    auto cell6Location = ophidian::util::Location(30, 10);
    addCell(cellStdCellMultirow, "cell6", cell6Location, 2, true);

    auto cell7Location = ophidian::util::Location(35, 10);
    addCell(cellStdCell, "cell7", cell7Location, 2, false);
}

MultirowAbacusFixture::MultirowAbacusFixture()
{
    design_.floorplan().chipOrigin(ophidian::util::Location(0, 0));
    design_.floorplan().chipUpperRightCorner(ophidian::util::Location(50, 40));
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(1.0, 10.0));
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 50, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 50, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 50, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 50, site);

    auto cellStdCell = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(cellStdCell, stdCellGeometry);

    auto cellStdCellMultirow = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCellMultirow, stdCellGeometryMR);
    design_.library().cellAlignment(cellStdCellMultirow, ophidian::placement::RowAlignment::EVEN);

    auto cell1Location = ophidian::util::Location(10, 28);
    addCell(cellStdCell, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(15, 28);
    addCell(cellStdCell, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(25, 22);
    addCell(cellStdCell, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(40, 30);
    addCell(cellStdCell, "cell4", cell4Location, 2, false);

    auto cell5Location = ophidian::util::Location(30, 0);
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
    design_.floorplan().chipOrigin(ophidian::util::Location(0, 0));
    design_.floorplan().chipUpperRightCorner(ophidian::util::Location(5, 4));
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(1.0, 1.0));
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 1.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 2.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 3.0), 5, site);

    cellStdCell_ = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 1))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(cellStdCell_, stdCellGeometry);

    auto cell1Location = ophidian::util::Location(1.0, 2.0);
    addCell(cellStdCell_, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(2.0, 0.0);
    addCell(cellStdCell_, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(3.0, 3.0);
    addCell(cellStdCell_, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(4.0, 1.0);
    addCell(cellStdCell_, "cell4", cell4Location, 2, false);

    auto fence1 = design_.fences().add("fence1");
    design_.fences().area(fence1, ophidian::util::MultiBox({ophidian::geometry::Box({0, 0}, {3, 3})}));

    auto cell1 = design_.netlist().find(ophidian::circuit::Cell(), "cell1");
    design_.fences().connect(fence1, cell1);
    design_.placement().cellFence(cell1, fence1);

    auto cell2 = design_.netlist().find(ophidian::circuit::Cell(), "cell2");
    design_.fences().connect(fence1, cell2);
    design_.placement().cellFence(cell2, fence1);
}

LargerLegalCircuitFixture::LargerLegalCircuitFixture()
{
    design_.floorplan().chipOrigin(ophidian::util::Location(0, 0));
    design_.floorplan().chipUpperRightCorner(ophidian::util::Location(50, 40));
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(5.0, 10.0));
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 10, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 10, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 10, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 10, site);

    cellStdCell_ = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(cellStdCell_, stdCellGeometry);

    auto cell1Location = ophidian::util::Location(10.0, 20.0);
    addCell(cellStdCell_, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(20.0, 0.0);
    addCell(cellStdCell_, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(30.0, 30.0);
    addCell(cellStdCell_, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(40.0, 10.0);
    addCell(cellStdCell_, "cell4", cell4Location, 2, false);
}

CircuitFixtureWithRandomCells::CircuitFixtureWithRandomCells(ophidian::util::Location chipOrigin, ophidian::util::Location chipUpperCorner, unsigned numberOfCells)
{
    design_.floorplan().chipOrigin(chipOrigin);
    design_.floorplan().chipUpperRightCorner(chipUpperCorner);
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(10.0, 10.0));
    unsigned numberOfRows = units::unit_cast<double>(chipUpperCorner.y())/10;
    unsigned sitesPerRow = units::unit_cast<double>(chipUpperCorner.x())/10;
    for (unsigned rowIndex = 0; rowIndex < numberOfRows; ++rowIndex) {
        design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, rowIndex*10), sitesPerRow, site);
    }

    auto singleRowStandardCell = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(singleRowStandardCell, stdCellGeometry);
    design_.library().cellAlignment(singleRowStandardCell, ophidian::placement::RowAlignment::NA);

    auto multiRowStandardCell = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(multiRowStandardCell, stdCellGeometryMR);
    design_.library().cellAlignment(multiRowStandardCell, ophidian::placement::RowAlignment::NA);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> xDistribution(units::unit_cast<double>(chipOrigin.x()), units::unit_cast<double>(chipUpperCorner.x()));
    std::uniform_int_distribution<int> yDistribution(units::unit_cast<double>(chipOrigin.y()), units::unit_cast<double>(chipUpperCorner.y()));
    std::uniform_real_distribution<double> stdCellDistribution(0, 1);
    for (unsigned cellIndex = 0; cellIndex < numberOfCells; ++cellIndex) {
        std::string cellName = "cell" + std::to_string(cellIndex);

        auto x = xDistribution(generator);
        auto y = yDistribution(generator);

        auto cellLocation = ophidian::util::Location(x, y);
        auto cellStdCell = (stdCellDistribution(generator) < 0.8) ? singleRowStandardCell : multiRowStandardCell;
//        auto cellStdCell = singleRowStandardCell;
        addCell(cellStdCell, cellName, cellLocation, 2, false);
    }
}

CircuitFixtureWithRandomAlignedCells::CircuitFixtureWithRandomAlignedCells(ophidian::util::Location chipOrigin, ophidian::util::Location chipUpperCorner, unsigned numberOfCells)
{
    float rowHeight = 10;

    design_.floorplan().chipOrigin(chipOrigin);
    design_.floorplan().chipUpperRightCorner(chipUpperCorner);
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(10.0, rowHeight));
    unsigned numberOfRows = units::unit_cast<double>(chipUpperCorner.y())/rowHeight;
    unsigned sitesPerRow = units::unit_cast<double>(chipUpperCorner.x())/rowHeight;
    for (unsigned rowIndex = 0; rowIndex < numberOfRows; ++rowIndex) {
        design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, rowIndex*rowHeight), sitesPerRow, site);
    }

    auto singleRowStandardCell = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(singleRowStandardCell, stdCellGeometry);
    design_.library().cellAlignment(singleRowStandardCell, ophidian::placement::RowAlignment::NA);

    auto multiRowStandardCell = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(multiRowStandardCell, stdCellGeometryMR);
    design_.library().cellAlignment(multiRowStandardCell, ophidian::placement::RowAlignment::NA);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> xDistribution(units::unit_cast<double>(chipOrigin.x()), units::unit_cast<double>(chipUpperCorner.x()));
    std::uniform_int_distribution<int> yDistribution(units::unit_cast<double>(chipOrigin.y()) / rowHeight, (units::unit_cast<double>(chipUpperCorner.y()) - rowHeight) / rowHeight);
    std::uniform_real_distribution<double> stdCellDistribution(0, 1);
    for (unsigned cellIndex = 0; cellIndex < numberOfCells; ++cellIndex) {
        std::string cellName = "cell" + std::to_string(cellIndex);

        auto x = xDistribution(generator);
        auto y = yDistribution(generator);

        auto cellLocation = ophidian::util::Location(x, y * rowHeight);
        auto cellStdCell = (stdCellDistribution(generator) < 0.8) ? singleRowStandardCell : multiRowStandardCell;
//        auto cellStdCell = singleRowStandardCell;
        addCell(cellStdCell, cellName, cellLocation, 2, false);
    }
}

ConstraintGraphCircuitFixture::ConstraintGraphCircuitFixture()
{
    design_.floorplan().chipOrigin(ophidian::util::Location(0, 0));
    design_.floorplan().chipUpperRightCorner(ophidian::util::Location(50, 40));
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(10.0, 10.0));
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 5, site);

    cellStdCell_ = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(cellStdCell_, stdCellGeometry);

    auto cell1Location = ophidian::util::Location(10.0, 20.0);
    addCell(cellStdCell_, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(18.0, 0.0);
    addCell(cellStdCell_, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(30.0, 30.0);
    addCell(cellStdCell_, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(25.0, 0.0);
    addCell(cellStdCell_, "cell4", cell4Location, 2, false);
}

ViolatingConstraintGraphCircuitFixture::ViolatingConstraintGraphCircuitFixture()
{
    design_.floorplan().chipOrigin(ophidian::util::Location(0, 0));
    design_.floorplan().chipUpperRightCorner(ophidian::util::Location(20, 20));
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(10.0, 10.0));
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 2, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 2, site);

    cellStdCell_ = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(cellStdCell_, stdCellGeometry);

    auto cell1Location = ophidian::util::Location(0, 0);
    addCell(cellStdCell_, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(5, 0);
    addCell(cellStdCell_, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(10, 0);
    addCell(cellStdCell_, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(15, 0);
    addCell(cellStdCell_, "cell4", cell4Location, 2, false);
}

MisalignedCircuitFixture::MisalignedCircuitFixture()
{
    design_.floorplan().chipOrigin(ophidian::util::Location(0, 0));
    design_.floorplan().chipUpperRightCorner(ophidian::util::Location(50, 40));
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(10.0, 10.0));
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 5, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 5, site);

    cellStdCell_ = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1");
    std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 10))};
    ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
    design_.library().geometry(cellStdCell_, stdCellGeometry);

    auto cell1Location = ophidian::util::Location(12.0, 22.0);
    addCell(cellStdCell_, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(18.0, 2.0);
    addCell(cellStdCell_, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(32.0, 28.0);
    addCell(cellStdCell_, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(38.0, 8.0);
    addCell(cellStdCell_, "cell4", cell4Location, 2, false);
}

ZeroSlackCircuitFixture::ZeroSlackCircuitFixture()
{
    design_.floorplan().chipOrigin(ophidian::util::Location(0, 0));
    design_.floorplan().chipUpperRightCorner(ophidian::util::Location(40, 50));
    auto site = design_.floorplan().add(ophidian::floorplan::Site(), "site", ophidian::util::Location(10.0, 10.0));
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 4, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 10.0), 4, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 20.0), 4, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 30.0), 4, site);
    design_.floorplan().add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 40.0), 4, site);

    cellStdCell_ = design_.standardCells().add(ophidian::standard_cell::Cell(), "INV_Z1_MR");
    std::vector<ophidian::geometry::Box> stdCellBoxesMR = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(10, 20))};
    ophidian::util::MultiBox stdCellGeometryMR(stdCellBoxesMR);
    design_.library().geometry(cellStdCell_, stdCellGeometryMR);
    design_.library().cellAlignment(cellStdCell_, ophidian::placement::RowAlignment::NA);

    auto cell1Location = ophidian::util::Location(0, 30.0);
    addCell(cellStdCell_, "cell1", cell1Location, 2, false);

    auto cell2Location = ophidian::util::Location(10.0, 20.0);
    addCell(cellStdCell_, "cell2", cell2Location, 3, false);

    auto cell3Location = ophidian::util::Location(20.0, 10.0);
    addCell(cellStdCell_, "cell3", cell3Location, 2, false);

    auto cell4Location = ophidian::util::Location(30.0, 0.0);
    addCell(cellStdCell_, "cell4", cell4Location, 2, false);
}
