#include "MultirowLegalization.h"

namespace ophidian {
namespace legalization {
MultirowLegalization::MultirowLegalization(design::Design & design)
    : mDesign(design), mSubrows(design),
      mInitialLocations(design.netlist().makeProperty<util::Location>(circuit::Cell())),
      mLegalLocations(design.netlist().makeProperty<util::Location>(circuit::Cell())),
      mCell2Cluster(design.netlist().makeProperty<Cluster>(circuit::Cell())),
      mCell2Subrow(design.netlist().makeProperty<Subrow>(circuit::Cell())),
      mClustersCells(mClusters),
      mClustersRightBoundary(mClusters),
      mSubrowsRightBoundary(mSubrows.makeProperty<util::micrometer_t>(Subrow()))
{
    for (auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
        auto cell = *cellIt;
        auto location = mDesign.placement().cellLocation(cell);

        mInitialLocations[cell] = location;
    }

    mDebug = false;
}

void MultirowLegalization::legalize(std::vector<circuit::Cell> cells, util::MultiBox legalizationArea)
{
    auto areaUpperRightCorner = legalizationArea[0].max_corner();

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

    for (auto subrow : mSubrows.range(Subrow())) {
        auto subrowOrigin = mSubrows.origin(subrow);
        mSubrowsRightBoundary[subrow] = subrowOrigin.x();
    }

//    auto box = legalizationArea[0];
//    std::cout << "legalization area " << box.min_corner().x() << ", " << box.min_corner().y()
//              << " -> " << box.max_corner().x() << ", " << box.max_corner().y() << std::endl;

//    for (auto subrow : mSubrows.range(Subrow())) {
//        auto subrowOrigin = mSubrows.origin(subrow);
//        auto subrowUpperCorner = mSubrows.upperCorner(subrow);
//        std::cout << "subrow " << subrowOrigin.toPoint().x() << ", " << subrowOrigin.toPoint().y()
//                  << " -> " << subrowUpperCorner.toPoint().x() << ", " << subrowUpperCorner.toPoint().y() << std::endl;
//    }

    unsigned cellIndex = 0;

    std::vector<circuit::Cell> legalCells;
    legalCells.reserve(cells.size());

    for (auto cellPair : sortedCells) {
//        if (cellIndex % 1000 == 0) {
//        }


        auto cell = cellPair.first;
        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        auto cellName = mDesign.netlist().name(cell);
//        if (cellName == "g558366") {
//            std::cout << "stop" << std::endl;
//        }

//        if (cellIndex == 243) {
//            std::cout << "stop " << std::endl;
//        }
        std::cout << "cell " << cellIndex << std::endl;
        std::cout << "cell " << cellName << std::endl;

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

                if (subrowOrigin.toPoint().y() + cellHeight > areaUpperRightCorner.y()) {
                    continue;
                }

                util::Location targetLocation(mInitialLocations[cell].x(), subrowOrigin.y());
                if (targetLocation.x() < subrowOrigin.x()) {
                    targetLocation.x(subrowOrigin.x());
                } else if (targetLocation.x() + util::micrometer_t(cellWidth) > subrowUpperCorner.x()) {
                    targetLocation.x(subrowUpperCorner.x() - util::micrometer_t(cellWidth));
                }

                auto cost = legalizeCell(cell, targetLocation, true);
                if (cost < bestCost) {
                    bestCost = cost;
                    bestSubrow = subrow;
                }

//                if (cellName == "h3b/u1_uk_K_r11_reg_36__u0") {
//                    std::cout << "origin " << subrowOrigin.x() << ", " << subrowOrigin.y() << std::endl;
//                    std::cout << "upper corner " << subrowUpperCorner.x() << ", " << subrowUpperCorner.y() << std::endl;
//                    std::cout << "location " << targetLocation.x() << ", " << targetLocation.y() << std::endl;

//                    std::cout << "cost " << cost << std::endl;
//                }
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

//        if (cellName == "h3a/g291245_u0") {
//        if (cellIndex >= 0) {
//            std::cout << cellIndex << std::endl;
//            std::cout << cellName << std::endl;
//            mDebug = true;
////            return;
//        }

        mCell2Subrow[cell] = bestSubrow;

        auto subrowOrigin = mSubrows.origin(bestSubrow);
        auto subrowUpperCorner = mSubrows.upperCorner(bestSubrow);


        util::Location targetLocation(mInitialLocations[cell].x(), subrowOrigin.y());
        if (targetLocation.x() < subrowOrigin.x()) {
            targetLocation.x(subrowOrigin.x());
        } else if (targetLocation.x() + util::micrometer_t(cellWidth) > subrowUpperCorner.x()) {
            targetLocation.x(subrowUpperCorner.x() - util::micrometer_t(cellWidth));
        }

        if (mDebug) {
            std::cout << "target location " << targetLocation.x() << ", " << targetLocation.y() << std::endl;
            std::cout << "target subrow origin " << subrowOrigin.x() << ", " << subrowOrigin.y() << std::endl;
            std::cout << "target subrow upper corner " << subrowUpperCorner.x() << ", " << subrowUpperCorner.y() << std::endl;
        }

        legalizeCell(cell, targetLocation, false);

        if (mDebug) {
            std::cout << "legal location " << mLegalLocations[cell].x() << ", " << mLegalLocations[cell].y() << std::endl;
        }

//        if (cellName == "h3a/g291245_u0") {
//        if (cellIndex >= 0) {
//            mDebug = false;
//        }


//        legalCells.push_back(cell);
//        bool boundaries = checkBoundaries(mDesign.floorplan(), mDesign.placement(), mDesign.placementMapping(), mDesign.netlist(), mDesign.fences(), legalCells);
//        std::cout << "check boundaries " << boundaries << std::endl;
//        if (!boundaries) {
//            std::cout << "violated check boundaries " << cellIndex << std::endl;
//            return;
//        }

//        if (cellIndex == 232) {
//            return;
//        }

        cellIndex++;
    }
}

util::micrometer_t MultirowLegalization::legalizeCell(circuit::Cell cell, util::Location targetLocation, bool trial)
{

    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
    auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
    auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();
    geometry::Box targetBox(targetLocation.toPoint(), geometry::Point(targetLocation.toPoint().x() + cellWidth, targetLocation.toPoint().y() + cellHeight));

    std::vector<Subrow> overlappingSubrows;
    mSubrows.findContainedSubrows(targetBox, overlappingSubrows);

    util::micrometer_t maxRightBoundary(-1);
    util::micrometer_t minUpperCorner(std::numeric_limits<double>::max());
    for (auto subrow : overlappingSubrows) {
        auto subrowRightBoundary = mSubrowsRightBoundary[subrow];
        auto subrowOrigin = mSubrows.origin(subrow);
        auto subrowUpperCorner = mSubrows.upperCorner(subrow);

        maxRightBoundary = std::max(maxRightBoundary, subrowRightBoundary);
        minUpperCorner = std::min(minUpperCorner, subrowUpperCorner.x());

        if (mDebug) {
            std::cout << "right boundary " << subrowRightBoundary << std::endl;
            std::cout << "subrow origin " << subrowOrigin.x() << ", " << subrowOrigin.y() << std::endl;
            std::cout << "subrow upper corner " << subrowUpperCorner.x() << ", " << subrowUpperCorner.y() << std::endl;
        }
    }

    if (maxRightBoundary > targetLocation.x()) {
        targetLocation.x(maxRightBoundary);
        targetBox.min_corner().x(units::unit_cast<double>(maxRightBoundary));
        targetBox.max_corner().x(units::unit_cast<double>(maxRightBoundary) + cellWidth);
    }

    if (targetLocation.x() + util::micrometer_t(cellWidth) <= minUpperCorner) {
        if (!trial) {
            auto cluster = mClusters.add();
            addCellToCluster(cell, cluster, targetLocation);

            mDesign.placement().placeCell(cell, mLegalLocations[cell]);

            geometry::Box targetBox(mLegalLocations[cell].toPoint(), geometry::Point(mLegalLocations[cell].toPoint().x() + cellWidth, mLegalLocations[cell].toPoint().y() + cellHeight));
            mCellsRtree.insert(RtreeNode(targetBox, cell));
        }

        if (mDebug) {
            std::cout << "no overlap" << std::endl;
            std::cout << "target location " << targetLocation.x() << ", " << targetLocation.y() << std::endl;
            std::cout << "legal location " << mLegalLocations[cell].x() << ", " << mLegalLocations[cell].y() << std::endl;
        }

        auto cost = util::micrometer_t(std::abs(units::unit_cast<double>(targetLocation.x() - mInitialLocations[cell].x())) +
                std::abs(units::unit_cast<double>(targetLocation.y() - mInitialLocations[cell].y())));
        return cost;
    } else {
        std::vector<RtreeNode> overlappingNodes;
        mCellsRtree.query(boost::geometry::index::intersects(targetBox), std::back_inserter(overlappingNodes));
        mCellsRtree.query(boost::geometry::index::within(targetBox), std::back_inserter(overlappingNodes));
        mCellsRtree.query(boost::geometry::index::contains(targetBox), std::back_inserter(overlappingNodes));
        util::micrometer_t minLeftSpace(std::numeric_limits<double>::max());
        Cluster minCluster;

        if (mDebug) {
            std::cout << "number of overlaps " << overlappingNodes.size() << std::endl;
        }

        std::vector<Cluster> overlappingClusters;
        for (auto node : overlappingNodes) {
            auto overlappingCell = node.second;
            auto overlappingCellName = mDesign.netlist().name(overlappingCell);
            auto overlappingCluster = mCell2Cluster[overlappingCell];

            if (std::find(overlappingClusters.begin(), overlappingClusters.end(), overlappingCluster) == overlappingClusters.end()) {
                overlappingClusters.push_back(overlappingCluster);

                auto leftSpace = clusterLeftSpace(overlappingCluster);

                if (mDebug) {
                    std::cout << "overlapping cell " << overlappingCellName << " " << node.first.min_corner().x() << ", " << node.first.min_corner().y() << std::endl;
                    std::cout << "left space " << leftSpace << std::endl;
                }

                if (leftSpace < minLeftSpace) {
                    minLeftSpace = leftSpace;
                    minCluster = overlappingCluster;

                    if (mDebug) {
                        std::cout << "updating min left space " << minLeftSpace << std::endl;
                    }
                }
            }

        }

        if (minLeftSpace < util::micrometer_t(cellWidth) && maxRightBoundary + util::micrometer_t(cellWidth) > minUpperCorner) {
            return util::micrometer_t(std::numeric_limits<double>::max());
        }

        if (mDebug) {
            std::cout << "target location " << targetLocation.x() << ", " << targetLocation.y() << std::endl;
        }

        if (!trial) {
//            if (mDebug) {
//                std::cout << "moving cluster to optimal location " << std::endl;
//            }
            mDesign.placement().placeCell(cell, targetLocation);
            addCellToCluster(cell, minCluster, targetLocation);
//            moveClusterToOptimalLocation(minCluster);
            if (mDebug) {
                std::cout << "merging " << overlappingClusters.size() << " clusters" << std::endl;
            }
            mergeClusters(overlappingClusters);
        }


        auto cost = util::micrometer_t(std::abs(units::unit_cast<double>(targetLocation.x() - mInitialLocations[cell].x())) +
                std::abs(units::unit_cast<double>(targetLocation.y() - mInitialLocations[cell].y())));
        return cost;
    }
}

void MultirowLegalization::addCellToCluster(circuit::Cell cell, MultirowLegalization::Cluster cluster, util::Location targetLocation)
{
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).toPoint().y();

    auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
    auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
    auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();
    auto cellHeightInRows = cellHeight / rowHeight;

    mClustersCells[cluster].push_back(cell);
    util::micrometer_t maxRightBoundary(0);
    for (auto rowIndex = 0; rowIndex < cellHeightInRows; rowIndex++) {
        auto yCoordinate = targetLocation.y() + util::micrometer_t(rowIndex * rowHeight);
        auto currentRightBoundary = mClustersRightBoundary[cluster][yCoordinate];
        maxRightBoundary = std::max(maxRightBoundary, currentRightBoundary);
        if (currentRightBoundary < targetLocation.x()) {
            mClustersRightBoundary[cluster][yCoordinate] = targetLocation.x() + util::micrometer_t(cellWidth);
        } else {
            mClustersRightBoundary[cluster][yCoordinate] = currentRightBoundary + util::micrometer_t(cellWidth);
        }

        auto subrow = mSubrows.findContainedSubrow(util::Location(currentRightBoundary, yCoordinate));
        auto subrowCapacity = mSubrows.capacity(subrow);
        mSubrows.capacity(subrow, subrowCapacity + util::micrometer_t(cellWidth));
        mSubrowsRightBoundary[subrow] = targetLocation.x() + util::micrometer_t(cellWidth);
    }

    mLegalLocations[cell] = (targetLocation.x() > maxRightBoundary) ? targetLocation : util::Location(maxRightBoundary, targetLocation.y());
    mCell2Cluster[cell] = cluster;
}

void MultirowLegalization::moveClusterToOptimalLocation(MultirowLegalization::Cluster cluster)
{
    auto site = *mDesign.floorplan().sitesRange().begin();
    auto siteWidth = mDesign.floorplan().siteUpperRightCorner(site).x();
    auto rowHeight = mDesign.floorplan().siteUpperRightCorner(site).y();

    auto clusterCells = mClustersCells[cluster];
    util::micrometer_t forceX(0);
//    util::micrometer_t forceY(0);
    util::micrometer_t minLeftMovement(-std::numeric_limits<double>::max());
    util::micrometer_t maxRightMovement(std::numeric_limits<double>::max());

    util::micrometer_t clusterLeftMovement(0);

    for (auto cell : clusterCells) {
        auto cellName = mDesign.netlist().name(cell);

        auto cellLegalLocation = mLegalLocations[cell];
        auto cellInitialLocation = mInitialLocations[cell];

        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();

        auto subrow = mSubrows.findContainedSubrow(cellLegalLocation);
        auto subrowOrigin = mSubrows.origin(subrow);
        auto subrowUpperCorner = mSubrows.upperCorner(subrow);

        auto leftDistance = subrowOrigin.x() - cellLegalLocation.x();
        auto rightDistance = subrowUpperCorner.x() - (cellLegalLocation.x() + util::micrometer_t(cellWidth));
        if (mDebug) {
            std::cout << "cell " << cellName << " legal location " << cellLegalLocation.x() << ", " << cellLegalLocation.y() << std::endl;
            std::cout << "left distance " << leftDistance << std::endl;
            std::cout << "right distance " << rightDistance << std::endl;
            std::cout << "subrow capacity " << mSubrows.capacity(subrow) << std::endl;
        }

        minLeftMovement = std::max(minLeftMovement, subrowOrigin.x() - cellLegalLocation.x());
        maxRightMovement = std::min(maxRightMovement, subrowUpperCorner.x() - (cellLegalLocation.x() + util::micrometer_t(cellWidth)));


        forceX = forceX + cellInitialLocation.x() - cellLegalLocation.x();
//        forceY = forceY + cellInitialLocation.y() - cellLegalLocation.y();

        auto clusterRightBoundary = mClustersRightBoundary[cluster][cellLegalLocation.y()];
        clusterLeftMovement = std::min(util::micrometer_t(0), subrowUpperCorner.x() - clusterRightBoundary);


        if (mDebug) {
            std::cout << "min left movement " << minLeftMovement << std::endl;
            std::cout << "max right movement " << maxRightMovement << std::endl;
            std::cout << "cluster right boundary " << clusterRightBoundary << std::endl;
        }
    }

    if (minLeftMovement > util::micrometer_t(0) && maxRightMovement < util::micrometer_t(0)) {
        std::cout << "can't place cluster!" << std::endl;
//        assert(false);
    }

    forceX = forceX / clusterCells.size();
//    forceY = forceY / clusterCells.size();

    forceX = std::round(units::unit_cast<double>(forceX / siteWidth)) * siteWidth;
//    forceY = std::round(units::unit_cast<double>(forceY / rowHeight)) * rowHeight;

    if (minLeftMovement <= util::micrometer_t(0) && maxRightMovement >= util::micrometer_t(0)) {
        forceX = std::max(minLeftMovement, std::min(maxRightMovement, forceX));
    } else if (minLeftMovement > util::micrometer_t(0)) {
        forceX = std::max(minLeftMovement, forceX);
    } else {
        forceX = std::min(maxRightMovement, forceX);
    }


    if (mDebug) {
        std::cout << "min left movement " << minLeftMovement << std::endl;
        std::cout << "max right movement " << maxRightMovement << std::endl;
        std::cout << "force x " << forceX << std::endl;
    }

    util::MultiBox clusterGeometry;
    for (auto cell : clusterCells) {
        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();
        auto cellHeight = cellGeometry.max_corner().y() - cellGeometry.min_corner().y();

        geometry::Box previousBox(mLegalLocations[cell].toPoint(), geometry::Point(mLegalLocations[cell].toPoint().x() + cellWidth, mLegalLocations[cell].toPoint().y() + cellHeight));
        RtreeNode previousNode(previousBox, cell);
        mCellsRtree.remove(previousNode);

//        mLegalLocations[cell] = util::Location(mLegalLocations[cell].x() + forceX, mLegalLocations[cell].y() + forceY);
        mLegalLocations[cell] = util::Location(mLegalLocations[cell].x() + forceX, mLegalLocations[cell].y());
        mDesign.placement().placeCell(cell, mLegalLocations[cell]);

        geometry::Box newBox(mLegalLocations[cell].toPoint(), geometry::Point(mLegalLocations[cell].toPoint().x() + cellWidth, mLegalLocations[cell].toPoint().y() + cellHeight));
        RtreeNode newNode(newBox, cell);
        mCellsRtree.insert(newNode);

        clusterGeometry.push_back(newBox);
    }

    auto clusterMultiPolygon = clusterGeometry.toMultiPolygon();
    std::vector<RtreeNode> overlappingNodes;
    mCellsRtree.query(boost::geometry::index::intersects(clusterMultiPolygon), std::back_inserter(overlappingNodes));
//    mCellsRtree.query(boost::geometry::index::within(clusterMultiPolygon), std::back_inserter(overlappingNodes));
//    mCellsRtree.query(boost::geometry::index::contains(clusterMultiPolygon), std::back_inserter(overlappingNodes));

    std::vector<Cluster> overlappingClusters;
    for (auto node : overlappingNodes) {
        auto overlappingCell = node.second;
        auto overlappingCluster = mCell2Cluster[overlappingCell];

        geometry::MultiPolygon intersectionPolygon;
        boost::geometry::intersection(clusterMultiPolygon, node.first, intersectionPolygon);
        auto intersectionArea = boost::geometry::area(intersectionPolygon);

        if (intersectionArea > 0 && overlappingCluster != cluster && std::find(overlappingClusters.begin(), overlappingClusters.end(), overlappingCluster) == overlappingClusters.end()) {
            overlappingClusters.push_back(overlappingCluster);
        }
    }

    if (overlappingClusters.size() > 0) {
        if (mDebug) {
            std::cout << "merging " << overlappingClusters.size() << " clusters" << std::endl;
        }
        overlappingClusters.push_back(cluster);
        mergeClusters(overlappingClusters);
    }
}

void MultirowLegalization::mergeClusters(std::vector<MultirowLegalization::Cluster> clusters)
{
    std::vector<std::pair<circuit::Cell, util::micrometer_t>> sortedCells;
    for (auto cluster : clusters) {
        for (auto cell : mClustersCells[cluster]) {
            auto cellLocation = mDesign.placement().cellLocation(cell);
            sortedCells.push_back(std::make_pair(cell, cellLocation.x()));
        }
    }
    std::sort(sortedCells.begin(), sortedCells.end(), CellPairComparator());

    auto newCluster = mClusters.add();

    for (auto cellPair : sortedCells) {
        auto cell = cellPair.first;
        addCellToCluster(cell, newCluster, mLegalLocations[cell]);
    }

    for (auto cluster : clusters) {
        mClusters.erase(cluster);
    }

    moveClusterToOptimalLocation(newCluster);
}

util::micrometer_t MultirowLegalization::clusterLeftSpace(MultirowLegalization::Cluster cluster)
{
    auto clusterCells = mClustersCells[cluster];

    util::micrometer_t minLeftMovement(-std::numeric_limits<double>::max());
    util::micrometer_t maxRightMovement(std::numeric_limits<double>::max());

    for (auto cell : clusterCells) {
        auto cellLegalLocation = mLegalLocations[cell];

        auto cellGeometry = mDesign.placementMapping().geometry(cell)[0];
        auto cellWidth = cellGeometry.max_corner().x() - cellGeometry.min_corner().x();

        auto subrow = mSubrows.findContainedSubrow(cellLegalLocation);
        auto subrowOrigin = mSubrows.origin(subrow);
        auto subrowUpperCorner = mSubrows.upperCorner(subrow);

        minLeftMovement = std::max(minLeftMovement, subrowOrigin.x() - cellLegalLocation.x());
        maxRightMovement = std::min(maxRightMovement, subrowUpperCorner.x() - (cellLegalLocation.x() + util::micrometer_t(cellWidth)));
    }

    return -minLeftMovement;
}

void MultirowLegalization::initialize(const std::vector<circuit::Cell> &cells, const util::MultiBox &legalizationArea)
{
    mSubrows.createSubrows(cells, legalizationArea);
    for (auto subrow : mSubrows.range(Subrow())) {
        auto subrowOrigin = mSubrows.origin(subrow);
        mSubrowsRightBoundary[subrow] = subrowOrigin.x();
    }
}

}
}
