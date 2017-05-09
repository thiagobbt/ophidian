#include <catch.hpp>

#include <ophidian/legalization/abacus.h>

namespace {
class LegalizationFixture
{
public:
    LegalizationFixture()
        : libraryMapping_(netlist_), placement_(netlist_),
        placementLibrary_(stdCells_), placementMapping_(placement_, placementLibrary_, netlist_, libraryMapping_) {
      floorplan_.chipOrigin(ophidian::util::Location(0, 0));
      floorplan_.chipUpperRightCorner(ophidian::util::Location(5, 5));
      auto site = floorplan_.add(ophidian::floorplan::Site(), "site", ophidian::util::Location(0.1, 1.0));
      floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 0.0), 50, site);
      floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 1.0), 50, site);
      floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 2.0), 50, site);
      floorplan_.add(ophidian::floorplan::Row(), ophidian::util::Location(0.0, 3.0), 50, site);

      auto cellStdCell = stdCells_.add(ophidian::standard_cell::Cell(), "INV_Z1");
      std::vector<ophidian::geometry::Box> stdCellBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(0, 0), ophidian::geometry::Point(1, 1))};
      ophidian::util::MultiBox stdCellGeometry(stdCellBoxes);
      placementLibrary_.geometry(cellStdCell, stdCellGeometry);

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
      addCell(cellStdCell, cell6Location, 2, true);

      auto cell7Location = ophidian::util::Location(3.5, 1.0);
      addCell(cellStdCell, cell7Location, 2, false);
    }

    void addCell(ophidian::standard_cell::Cell stdCell, ophidian::util::Location cellLocation, unsigned numberOfPins, bool fixed) {
        auto cell = netlist_.add(ophidian::circuit::Cell());
        for (unsigned pinIndex = 0; pinIndex < numberOfPins; ++pinIndex) {
            auto cellPin = netlist_.add(ophidian::circuit::Pin());
            netlist_.add(cell, cellPin);
        }
        libraryMapping_.cellStdCell(cell, stdCell);
        placement_.placeCell(cell, cellLocation);
        placement_.fixLocation(cell, fixed);
    }

public:
    ophidian::circuit::Netlist netlist_;
    ophidian::standard_cell::StandardCells stdCells_;
    ophidian::placement::Library placementLibrary_;
    ophidian::circuit::LibraryMapping libraryMapping_;
    ophidian::floorplan::Floorplan floorplan_;
    ophidian::placement::Placement placement_;
    ophidian::placement::PlacementMapping placementMapping_;
};
}

TEST_CASE_METHOD(LegalizationFixture, "Legalization: legalizing small circuit using Abacus", "[legalization][abacus]") {
    std::vector<ophidian::util::Location> expectedLocations = {
        ophidian::util::Location(0.7, 3.0),
        ophidian::util::Location(1.7, 3.0),
        ophidian::util::Location(2.5, 2.0),
        ophidian::util::Location(4.0, 3.0),
        ophidian::util::Location(3.0, 0.0),
        ophidian::util::Location(3.0, 1.0),
        ophidian::util::Location(4.0, 1.0),
    };

    ophidian::legalization::Abacus abacus(netlist_, floorplan_, placement_, placementMapping_);
    abacus.legalizePlacement();

    std::vector<ophidian::util::Location> cellLocations;
    for (auto cellIt = netlist_.begin(ophidian::circuit::Cell()); cellIt != netlist_.end(ophidian::circuit::Cell()); ++cellIt) {
        cellLocations.push_back(placement_.cellLocation(*cellIt));
    }

    REQUIRE(cellLocations.size() == expectedLocations.size());
    REQUIRE(std::is_permutation(expectedLocations.begin(), expectedLocations.end(), cellLocations.begin()));
}
