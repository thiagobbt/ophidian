#include "BinDecomposition.h"

namespace ophidian {
namespace legalization {
BinDecomposition::BinDecomposition(design::Design &design)
    : mDesign(design),
      mBinsCells(mBins), mBinsBoxes(mBins)
{

}

void BinDecomposition::decomposeCircuitInBins(util::MultiBox area, std::vector<circuit::Cell> &cells, unsigned binSizeInRows)
{
    mBins.clear();
    mBinsRtree.clear();

    double rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();
    double binSize = binSizeInRows * rowHeight;

    for (auto box : area) {
        double boxWidth = box.max_corner().x() - box.min_corner().x();
        double boxHeight = box.max_corner().y() - box.min_corner().y();
        unsigned numberOfBinsInX = std::ceil(boxWidth / binSize);
        unsigned numberOfBinsInY = std::ceil(boxHeight / binSize);
        for (unsigned binIndexX = 0; binIndexX < numberOfBinsInX; binIndexX++) {
            for (unsigned binIndexY = 0; binIndexY < numberOfBinsInY; binIndexY++) {
                auto bin = mBins.add();

                util::Location binLocation(box.min_corner().x() + binIndexX * binSize, box.min_corner().y() + binIndexY * binSize);
                util::Location binUpperCorner(binLocation.toPoint().x() + binSize, binLocation.toPoint().y() + binSize);
                geometry::Box binBox(binLocation.toPoint(), binUpperCorner.toPoint());
                geometry::Box intersectionBox;
                boost::geometry::intersection(binBox, box, intersectionBox);
                binBox = intersectionBox;

                RtreeNode binNode(binBox, bin);
                mBinsRtree.insert(binNode);
                mBinsBoxes[bin] = binBox;
            }
        }
    }

    for (auto cell : cells) {
        if (!mDesign.placement().isFixed(cell)) {
            auto cellBox = mDesign.placementMapping().geometry(cell)[0];
            std::vector<RtreeNode> intersectingBins;
            mBinsRtree.query(boost::geometry::index::intersects(cellBox), std::back_inserter(intersectingBins));
            Bin chosenBin;
            if (intersectingBins.empty()) {
                mBinsRtree.query(boost::geometry::index::nearest(cellBox, 1), std::back_inserter(intersectingBins));
                chosenBin = intersectingBins.front().second;
            } else {
                double maxIntersection = 0;
                for (auto binNode : intersectingBins) {
                    auto binBox = binNode.first;
                    auto bin = binNode.second;

                    geometry::Box intersectionBox;
                    boost::geometry::intersection(cellBox, binBox, intersectionBox);

                    double intersectionArea = boost::geometry::area(intersectionBox);
                    if (intersectionArea > maxIntersection) {
                        maxIntersection = intersectionArea;
                        chosenBin = bin;
                    }
                }
            }

            mBinsCells[chosenBin].push_back(cell);
        }
    }

    std::cout << "number of bins " << mBins.size() << std::endl;
    std::cout << "number of boxes " << area.size() << std::endl;

    unsigned maximumNumberOfCellsPerBin = 0;
    for (auto bin : mBins) {
        auto binBox = mBinsBoxes[bin];
        unsigned numberOfCells = mBinsCells[bin].size();

        auto binWidth = binBox.max_corner().x() - binBox.min_corner().x();

//        std::cout << "bin width " << binWidth << std::endl;

        maximumNumberOfCellsPerBin = std::max(numberOfCells, maximumNumberOfCellsPerBin);

        auto binArea = boost::geometry::area(binBox);
        auto cellArea = 0.0;
        for (auto cell : mBinsCells[bin]) {
            auto cellBox = mDesign.placementMapping().geometry(cell)[0];
            cellArea += boost::geometry::area(cellBox);

            auto cellWidth = cellBox.max_corner().x() - cellBox.min_corner().x();
            if (cellWidth > binWidth) {
                std::cout << "cell width " << cellWidth << " greater than bin width " << std::endl;
            }
        }
//        std::cout << "bin area " << binArea << std::endl;
//        std::cout << "cells area " << cellArea << std::endl;
    }
    std::cout << "maximum number of cells per bin " << maximumNumberOfCellsPerBin << std::endl;
}

std::size_t BinDecomposition::size(Bin)
{
    return mBins.size();
}

ophidian::util::Range<BinDecomposition::BinsIterator> BinDecomposition::range(Bin) const
{
    return ophidian::util::Range<BinsIterator>(mBins.begin(), mBins.end());
}
}
}
