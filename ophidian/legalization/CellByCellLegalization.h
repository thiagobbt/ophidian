#ifndef CELLBYCELLLEGALIZATION_H
#define CELLBYCELLLEGALIZATION_H

#include <ophidian/design/Design.h>
#include <ophidian/legalization/Subrows.h>

#include <ophidian/legalization/LegalizationCheck.h>

#include <ophidian/util/Debug.h>

namespace ophidian {
namespace legalization {
template <class LegalizationEngine>
class CellByCellLegalization
{
public:
    class CellPairComparator
    {
public:
        bool operator()(const std::pair<circuit::Cell, util::micrometer_t> & cellPair1, const std::pair<circuit::Cell, util::micrometer_t> & cellPair2) {
            return cellPair1.second < cellPair2.second;
        }
    };

    CellByCellLegalization(design::Design & design)
        : mDesign(design), mSubrows(design), mLegalizationEngine(design),
          mCell2Subrow(design.netlist().makeProperty<Subrow>(circuit::Cell())),
          mInitialLocations(design.netlist().makeProperty<util::Location>(circuit::Cell()))
    {
        for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
            auto cell = *cellIt;
            auto location = mDesign.placement().cellLocation(cell);

            mInitialLocations[cell] = location;
        }
    }

    void legalize(const std::vector<circuit::Cell> & cells, const ophidian::util::MultiBox & legalizationArea) {
        auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

        std::vector<std::pair<circuit::Cell, util::micrometer_t>> sortedCells;
        for (auto cell : cells) {
            if (!mDesign.placement().isFixed(cell)) {
                auto cellLocation = mDesign.placement().cellLocation(cell);
                sortedCells.push_back(std::make_pair(cell, cellLocation.x()));
            }
        }
        std::sort(sortedCells.begin(), sortedCells.end(), CellPairComparator());

        mSubrows.createSubrows(cells, legalizationArea);

        mLegalizationEngine.initialize(cells, legalizationArea);

        std::vector<circuit::Cell> legalCells;

        unsigned cellIndex = 0;
        for (auto cellPair : sortedCells) {
            auto cell = cellPair.first;
            auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
            auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
            auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

            auto cellName = mDesign.netlist().name(cell);

//            if (cellIndex % 1000 == 0) {
                std::cout << "cell " << cellIndex << std::endl;
                std::cout << "cell " << cellName << std::endl;
//            }

//            std::cout << "cell " << cellIndex << std::endl;
//            std::cout << "cell " << cellName << std::endl;

//            if (cellIndex == 5) {
//                util::Debug::mDebug = true;
//            }

            auto cellAlignment = mDesign.placementMapping().alignment(cell);

            unsigned rowsToSearch = 5;

            util::micrometer_t bestCost(std::numeric_limits<double>::max());
            Subrow bestSubrow;
            while (units::unit_cast<double>(bestCost) == std::numeric_limits<double>::max()) {
                std::vector<Subrow> closeSubrows;
                mSubrows.findClosestSubrows(rowsToSearch, mInitialLocations[cell], closeSubrows);

                for (auto subrow : closeSubrows) {
                    auto subrowOrigin = mSubrows.origin(subrow);
                    auto subrowUpperCorner = mSubrows.upperCorner(subrow);

                    placement::RowAlignment subrowAlignment = (((int)(subrowOrigin.toPoint().y() / rowHeight) % 2) == 0) ? placement::RowAlignment::EVEN : placement::RowAlignment::ODD;
                    if (cellAlignment != placement::RowAlignment::NA && cellAlignment != subrowAlignment) {
                        continue;
                    }

                    util::Location targetLocation = adjustLocationToSubrow(mInitialLocations[cell], cellWidth, subrow);

                    if (util::Debug::mDebug) {
                        std::cout << "subrow origin " << subrowOrigin.x() << ", " << subrowOrigin.y() << std::endl;
                        std::cout << "subrow upper corner " << subrowUpperCorner.x() << ", " << subrowUpperCorner.y() << std::endl;
                    }

                    geometry::Box targetBox(targetLocation.toPoint(), geometry::Point(targetLocation.toPoint().x() + cellWidth, targetLocation.toPoint().y() + cellHeight));
                    if (!mSubrows.isInsideSubrows(targetBox)) {
                        if (util::Debug::mDebug) {
                            std::cout << "outside subrows region " << std::endl;
                        }
                        continue;
                    }

                    auto cost = mLegalizationEngine.legalizeCell(cell, targetLocation, true);
                    if (util::Debug::mDebug) {
                        std::cout << "cost " << cost << std::endl;
                    }
                    if (cost < bestCost) {
                        bestCost = cost;
                        bestSubrow = subrow;
                    }
                }

                if (rowsToSearch == mSubrows.rowCount() && units::unit_cast<double>(bestCost) == std::numeric_limits<double>::max()) {
                    std::cout << "could not legalize cell " << cellName << std::endl;
                    return;
                }

                rowsToSearch *= 2;
                if (rowsToSearch > mSubrows.rowCount()) {
                    rowsToSearch = mSubrows.rowCount();
                }
            }

            mCell2Subrow[cell] = bestSubrow;

            util::Location targetLocation = adjustLocationToSubrow(mInitialLocations[cell], cellWidth, bestSubrow);

            mLegalizationEngine.legalizeCell(cell, targetLocation, false);

            legalCells.push_back(cell);
            bool boundaries = legalization::checkBoundaries(mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping(), mDesign.netlist(), mDesign.fences(), legalCells);
            if (!boundaries) {
                std::cout << "cell " << cellIndex << " violated boundaries" << std::endl;
                return;
            }

            cellIndex++;
        }
    }

    util::Location adjustLocationToSubrow(util::Location cellLocation, double cellWidth, Subrow subrow) {
        auto subrowOrigin = mSubrows.origin(subrow);
        auto subrowUpperCorner = mSubrows.upperCorner(subrow);

        util::Location targetLocation(cellLocation.x(), subrowOrigin.y());
        if (targetLocation.x() < subrowOrigin.x()) {
            targetLocation.x(subrowOrigin.x());
        } else if (targetLocation.x() + util::micrometer_t(cellWidth) > subrowUpperCorner.x()) {
            targetLocation.x(subrowUpperCorner.x() - util::micrometer_t(cellWidth));
        }
        return targetLocation;
    }

private:
    design::Design & mDesign;

    Subrows mSubrows;

    LegalizationEngine mLegalizationEngine;

    entity_system::Property<circuit::Cell, Subrow> mCell2Subrow;
    entity_system::Property<circuit::Cell, util::Location> mInitialLocations;
};
}
}

#endif // CELLBYCELLLEGALIZATION_H
