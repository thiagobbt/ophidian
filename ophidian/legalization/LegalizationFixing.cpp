#include "LegalizationFixing.h"

namespace ophidian {
namespace legalization {
LegalizationFixing::LegalizationFixing(design::Design &design)
    : mDesign(design),
      mInitialLocations(mDesign.netlist().makeProperty<util::Location>(circuit::Cell())),
      mInitialFixed(mDesign.netlist().makeProperty<bool>(circuit::Cell())),
      mLegalizer(design)
{
    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); cellIt++) {
        mInitialLocations[*cellIt] = mDesign.placement().cellLocation(*cellIt);
        mInitialFixed[*cellIt] = mDesign.placement().isFixed(*cellIt);
    }
}

void LegalizationFixing::fixLegalization(const std::vector<circuit::Cell> &cells, geometry::Box area)
{
//    legalization::ILPLegalizationWithConstraintGraph legalizer(mDesign);

    std::vector<circuit::Cell> legalizedCells;
    legalizedCells.reserve(mDesign.netlist().size(circuit::Cell()));
    std::vector<std::pair<circuit::Cell, double>> unlegalizedCells;
    unlegalizedCells.reserve(mDesign.netlist().size(circuit::Cell()));

    for (auto cell : cells) {
        auto cellName = mDesign.netlist().name(cell);

//        std::cout << "cell " << cellName << std::endl;

        auto cellLocation = mDesign.placement().cellLocation(cell);
        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        auto initialLocation = mInitialLocations[cell];
        auto displacement = std::abs(cellLocation.toPoint().x() - initialLocation.toPoint().x()) + std::abs(cellLocation.toPoint().y() - initialLocation.toPoint().y());
        if (!mDesign.placement().isFixed(cell) && displacement < 0.1) {
//            std::cout << "unlegalized " << std::endl;
            unlegalizedCells.push_back(std::make_pair(cell, cellHeight));
        } else {
//            std::cout << "legalized " << std::endl;
            legalizedCells.push_back(cell);
        }
    }
    legalizedCells.shrink_to_fit();
    unlegalizedCells.shrink_to_fit();

    mLegalizer.buildRtree(legalizedCells);

    std::cout << "number of legalized cells " << legalizedCells.size() << std::endl;
    std::cout << "number of unlegalized cells " << unlegalizedCells.size() << std::endl;

    std::vector<std::pair<circuit::Cell, double>> newUnlegalizedCells;
//    placeCellsOverOtherCells(unlegalizedCells, legalizedCells, area, newUnlegalizedCells);
    placeCellsInSubrows(unlegalizedCells, legalizedCells, area, newUnlegalizedCells);

    std::cout << "remaining cells to legalize " << newUnlegalizedCells.size() << std::endl;
    std::cout << "trying step 2 " << std::endl;

    std::vector<std::pair<circuit::Cell, double>> finalUnlegalizedCells(newUnlegalizedCells);
////    placeCellsInSubrows(newUnlegalizedCells, legalizedCells, area, finalUnlegalizedCells);
    placeCellsOverOtherCells(newUnlegalizedCells, legalizedCells, area, finalUnlegalizedCells);

    std::cout << "final unlegalized cells " << finalUnlegalizedCells.size() << std::endl;
    if (finalUnlegalizedCells.size() == 0) {
        std::cout << "fixed :)" << std::endl;
    } else {
        std::cout << "not fixed :(" << std::endl;
    }
}

void LegalizationFixing::placeCellsOverOtherCells(std::vector<std::pair<circuit::Cell, double>> &unlegalizedCells, std::vector<circuit::Cell> &legalizedCells, geometry::Box area, std::vector<std::pair<circuit::Cell, double> > & newUnlegalizedCells)
{
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

//    legalization::ConstraintGraph<legalization::LeftComparator> constraintGraph(mDesign);

//    constraintGraph.buildConstraintGraph(legalizedCells, util::micrometer_t(area.min_corner().x()), util::micrometer_t(area.max_corner().x()));
//    constraintGraph.removeTransitiveEdges();

    newUnlegalizedCells.clear();
    newUnlegalizedCells.reserve(unlegalizedCells.size());

    std::sort(unlegalizedCells.begin(), unlegalizedCells.end(), CellPairComparator());

    unsigned cellIndex = 0;
    for (auto unlegalizedCellPair : unlegalizedCells) {
        auto unlegalizedCell = unlegalizedCellPair.first;

        auto unlegalizedCellName = mDesign.netlist().name(unlegalizedCell);
        auto unlegalizedCellGeometry = mDesign.placementMapping().geometry(unlegalizedCell)[0];
        auto cellWidth = unlegalizedCellGeometry.max_corner().x() - unlegalizedCellGeometry.min_corner().x();
        auto cellHeight = unlegalizedCellGeometry.max_corner().y() - unlegalizedCellGeometry.min_corner().y();
        auto cellAlignment = mDesign.placementMapping().alignment(unlegalizedCell);

//        std::cout << "unlegalized cell index " << cellIndex << std::endl;
//        std::cout << "unlegalized cell name " << unlegalizedCellName << std::endl;

        auto unlegalizedCellBox = mDesign.placementMapping().geometry(unlegalizedCell)[0];
        auto unlegalizedCellWidth = unlegalizedCellBox.max_corner().x() - unlegalizedCellBox.min_corner().x();

//        legalizedCells.push_back(unlegalizedCell);

        bool result = false;
        for (auto legalizedCell : legalizedCells) {
            auto legalizedCellLocation = mDesign.placement().cellLocation(legalizedCell);
//            auto legalizedCellSlack = constraintGraph.slack(legalizedCell);
//            if (legalizedCellSlack < unlegalizedCellWidth) {
//                continue;
//            }

            placement::RowAlignment locationAlignment = (((int)(legalizedCellLocation.toPoint().y() / rowHeight) % 2) == 0) ? placement::RowAlignment::EVEN : placement::RowAlignment::ODD;
            if (cellAlignment != placement::RowAlignment::NA && cellAlignment != locationAlignment) {
                continue;
            }

            if (legalizedCellLocation.toPoint().x() + cellWidth > area.max_corner().x() || legalizedCellLocation.toPoint().y() + cellHeight > area.max_corner().y()) {
                continue;
            }

            result |= mLegalizer.legalizeCell(unlegalizedCell, legalizedCellLocation.toPoint(), legalizedCells, area, false);
            if (result) {
                break;
            }
        }

        if (!result) {
//            std::cout << "could not legalized cell " << unlegalizedCellName << std::endl;
            newUnlegalizedCells.push_back(unlegalizedCellPair);
        } else {
//            std::cout << "legalized cell " << unlegalizedCellName << std::endl;
            legalizedCells.push_back(unlegalizedCell);
//            constraintGraph.addCell(unlegalizedCell);
//            constraintGraph.removeTransitiveEdges();
//            constraintGraph.calculateSlacks(area.min_corner().x(), area.max_corner().x());
//            auto worstSlack = constraintGraph.worstSlack();
//            std::cout << "worst slack " << worstSlack << std::endl;
        }

        cellIndex++;
    }
}

void LegalizationFixing::placeCellsInSubrows(std::vector<std::pair<circuit::Cell, double> > &unlegalizedCells, std::vector<circuit::Cell> &legalizedCells, geometry::Box area, std::vector<std::pair<circuit::Cell, double> > &newUnlegalizedCells)
{
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    for (auto cell : legalizedCells) {
        mDesign.placement().fixLocation(cell, true);
    }

    Subrows subrows(mDesign);
    util::MultiBox multiBoxArea({area});
    subrows.createSubrows(legalizedCells, multiBoxArea);

    for (auto cell : legalizedCells) {
        mDesign.placement().fixLocation(cell, mInitialFixed[cell]);
    }

//    if (unlegalizedCells.size() > 0) {
//        for (auto subrow : subrows.range(Subrow())) {
//            auto subrowOrigin = subrows.origin(subrow);
//            auto subrowUpperCorner = subrows.upperCorner(subrow);
//            std::cout << "subrow " << subrowOrigin.x() << ", " << subrowOrigin.y() << " -> " << subrowUpperCorner.x() << ", " << subrowUpperCorner.y() << std::endl;
//        }
//    }

    newUnlegalizedCells.clear();
    newUnlegalizedCells.reserve(unlegalizedCells.size());

    std::sort(unlegalizedCells.begin(), unlegalizedCells.end(), CellPairComparator());

    unsigned cellIndex = 0;
    for (auto unlegalizedCellPair : unlegalizedCells) {
        auto unlegalizedCell = unlegalizedCellPair.first;

        auto unlegalizedCellName = mDesign.netlist().name(unlegalizedCell);
        auto unlegalizedCellGeometry = mDesign.placementMapping().geometry(unlegalizedCell)[0];
        auto cellWidth = unlegalizedCellGeometry.max_corner().x() - unlegalizedCellGeometry.min_corner().x();
        auto cellHeight = unlegalizedCellGeometry.max_corner().y() - unlegalizedCellGeometry.min_corner().y();
        auto cellAlignment = mDesign.placementMapping().alignment(unlegalizedCell);

//        std::cout << "unlegalized cell index " << cellIndex << std::endl;
//        std::cout << "unlegalized cell name " << unlegalizedCellName << std::endl;

//        if (unlegalizedCellName == "h3b/u1_uk_K_r11_reg_35__u0") {
//            std::cout << "stop " << std::endl;
//            mDesign.writeDefFile("test.def", legalizedCells);
//        }

        bool result = false;
        for (auto subrow : subrows.range(Subrow())) {
            auto subrowOrigin = subrows.origin(subrow);

            placement::RowAlignment locationAlignment = (((int)(subrowOrigin.toPoint().y() / rowHeight) % 2) == 0) ? placement::RowAlignment::EVEN : placement::RowAlignment::ODD;
            if (cellAlignment != placement::RowAlignment::NA && cellAlignment != locationAlignment) {
                continue;
            }

            if (subrowOrigin.toPoint().x() + cellWidth > area.max_corner().x() || subrowOrigin.toPoint().y() + cellHeight > area.max_corner().y()) {
                continue;
            }

            result |= mLegalizer.legalizeCell(unlegalizedCell, subrowOrigin.toPoint(), legalizedCells, area, false);
            if (result) {
                break;
            }
        }

        if (!result) {
//            std::cout << "could not legalized cell " << unlegalizedCellName << std::endl;
            newUnlegalizedCells.push_back(unlegalizedCellPair);
        } else {
//            std::cout << "legalized cell " << unlegalizedCellName << std::endl;
            legalizedCells.push_back(unlegalizedCell);

            for (auto legalizedCell : legalizedCells) {
                mDesign.placement().fixLocation(legalizedCell, true);
            }
            subrows.createSubrows(legalizedCells, multiBoxArea);
            for (auto legalizedCell : legalizedCells) {
                mDesign.placement().fixLocation(legalizedCell, mInitialFixed[legalizedCell]);
            }

//            bool result = checkCellOverlaps(mDesign.placementMapping(), mDesign.netlist(), legalizedCells);
//            if (!result) {
//                std::cout << "cell " << cellIndex << " " << unlegalizedCellName << " violated overlap" << std::endl;
////                mDesign.writeDefFile("test.def", legalizedCells);
//            }
        }
        cellIndex++;
    }
}

}
}
