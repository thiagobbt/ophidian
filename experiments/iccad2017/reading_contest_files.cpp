#include <catch.hpp>
#include <iostream>
#include "iccad2017_wrapper.h"


TEST_CASE("open circuit: pci_bridge32_a_md1", "[iccad2017]")
{
    iccad2017_wrapper iccad("./input_files/ICCAD2017/pci_bridge32_a_md1");
    std::cout << "Circuit: pci_bridge32_a_md1" << std::endl;

    //test number of cells
    std::cout << "Number of cells: " << iccad.mNetlist.size(ophidian::circuit::Cell()) << std::endl;
    REQUIRE(iccad.mNetlist.size(ophidian::circuit::Cell()) == 29521);

    //test cell pins
    auto stdCellin01f01X2HO = iccad.mStdCells.find(ophidian::standard_cell::Cell(), "in01f01X2HO");
    auto pincellin01f01X2HOa = iccad.mStdCells.find(ophidian::standard_cell::Pin(), "in01f01X2HO:a");
    REQUIRE(iccad.mStdCells.owner(pincellin01f01X2HOa) == stdCellin01f01X2HO);
    REQUIRE(iccad.mStdCells.pins(stdCellin01f01X2HO).size() == 4);

    //test cell position
    ophidian::circuit::Cell netListCell = iccad.mNetlist.find(ophidian::circuit::Cell(), "FE_OFC1556_n_4732");
    ophidian::util::Location location = iccad.mPlacement.cellLocation(netListCell);
    std::cout << "Cell FE_OFC1556_n_4732 location is (" << location.x() << ", " << location.y() << ")" << std::endl;
    ophidian::util::Location expectedLocation(172444, 370471);
    REQUIRE(expectedLocation == location);

    //test cell geometry
    ophidian::util::MultiBox geometry = iccad.mPlacementMapping.geometry(netListCell);
    std::vector<ophidian::geometry::Box> cellExpectedBoxes = {ophidian::geometry::Box(ophidian::geometry::Point(172444, 370471), ophidian::geometry::Point(0.8*1000 + 172444, 4*1000 + 370471))};
    ophidian::util::MultiBox cellExpectedGeometry(cellExpectedBoxes);
    REQUIRE(geometry == cellExpectedGeometry);

    //test chip boundaries
    std::cout << "Chip Origin (" << iccad.mFloorplan.chipOrigin().x() << ", " << iccad.mFloorplan.chipOrigin().y() << ")" << std::endl;
    std::cout << "Chip Upper Right Corner (" << iccad.mFloorplan.chipUpperRightCorner().x() << ", " << iccad.mFloorplan.chipUpperRightCorner().y() << ")" << std::endl;
    ophidian::util::Location expectedChipOrigin(0, 0);
    ophidian::util::Location expectedChipUpperRightCorner(400000, 400000);
    REQUIRE(iccad.mFloorplan.chipOrigin() == expectedChipOrigin);
    REQUIRE(iccad.mFloorplan.chipUpperRightCorner() == expectedChipUpperRightCorner);

    //test sites
    std::cout << "Site (" << iccad.mFloorplan.siteUpperRightCorner(*iccad.mFloorplan.sitesRange().begin()).x() << ", " << iccad.mFloorplan.siteUpperRightCorner(*iccad.mFloorplan.sitesRange().begin()).y() << ")" << std::endl;
    ophidian::util::Location expectedSiteSize(200, 2000);
    REQUIRE(iccad.mFloorplan.siteUpperRightCorner(*iccad.mFloorplan.sitesRange().begin()) == expectedSiteSize);

    //row size
    auto row = *iccad.mFloorplan.rowsRange().begin();
    std::cout << "Row size (" << iccad.mFloorplan.rowUpperRightCorner(row).x() << ", " << iccad.mFloorplan.rowUpperRightCorner(row).y() << ")" << std::endl;
    ophidian::util::Location expectedRowUpperRigthCorner(200*2000, 2000);
    REQUIRE(iccad.mFloorplan.rowUpperRightCorner(row) == expectedRowUpperRigthCorner);
}
