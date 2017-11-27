#include "CheckPerturbation.h"

namespace ophidian {
namespace legalization {
CheckPerturbation::CheckPerturbation(design::Design &design, Subrows &subrows, std::vector<circuit::Cell> &cells)
    : mDesign(design), mSubrows(subrows),
        mSubrowsMultirowCells(mSubrows.makeProperty<std::vector<circuit::Cell>>(Subrow())) {
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();
    for (auto cell : cells) {
        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();
        if (cellHeight > rowHeight) {
            auto subrow = mSubrows.findContainedSubrow(cellGeometry);
            mSubrowsMultirowCells[subrow].push_back(cell);
        }
    }
}

int CheckPerturbation::numberOfPerturbedLines(Subrow subrow)
{
    unsigned numberOfLines = 0;
    std::queue<Subrow> subrowsToVisit;
    subrowsToVisit.push(subrow);
    std::vector<Subrow> visitedSubrows;
    visitedSubrows.push_back(subrow);
    while (!subrowsToVisit.empty()) {
        auto currentSubrow = subrowsToVisit.front();
        subrowsToVisit.pop();
        numberOfLines++;
        for (auto multirowCell : mSubrowsMultirowCells[currentSubrow]) {
            auto cellGeometry = mDesign.placementMapping().geometry(multirowCell)[0];
            std::vector<Subrow> otherSubrows;
            mSubrows.findContainedSubrows(cellGeometry, otherSubrows);
            for (auto subrow : otherSubrows) {
                if (std::find(visitedSubrows.begin(), visitedSubrows.end(), subrow) == visitedSubrows.end()) {
                    subrowsToVisit.push(subrow);
                    visitedSubrows.push_back(subrow);
                }
            }
        }
    }

    return numberOfLines;
}

}
}
