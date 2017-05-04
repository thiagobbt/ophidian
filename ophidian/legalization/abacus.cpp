#include "abacus.h"

namespace ophidian {
namespace legalization {
Abacus::Abacus(const circuit::Netlist & netlist, const floorplan::Floorplan & floorplan, placement::Placement & placement, const placement::PlacementMapping & placementMapping)
    : netlist_(netlist), floorplan_(floorplan), placement_(placement), placementMapping_(placementMapping),
    subrowOrigins_(subrows_), subrowUpperCorners_(subrows_), subrowCells_(subrows_, abacusCells_),
    cellInitialLocations_(abacusCells_), cellLegalLocations_(abacusCells_), cellWidths_(abacusCells_), cellWeights_(abacusCells_),
    clusterOrigins_(clusters_), clusterWidths_(clusters_), clusterWeights_(clusters_), clusterDisplacements_(clusters_), clusterLastCells_(clusters_){
    createSubrows();
}

void Abacus::legalizePlacement()
{
    std::vector<std::pair<AbacusCell, util::Location>> sortedCells;
    sortedCells.reserve(netlist_.size(circuit::Cell()));
    for (auto cell_it = netlist_.begin(circuit::Cell()); cell_it != netlist_.end(circuit::Cell()); ++cell_it) {
        auto abacus_cell = abacusCells_.add();
        abacusCell2NetlistCell_[abacus_cell] = *cell_it;
        cellInitialLocations_[abacus_cell] = placement_.cellLocation(*cell_it);
        auto cellGeometry = placementMapping_.geometry(*cell_it);
        cellWidths_[abacus_cell] = ophidian::util::micrometer_t(cellGeometry[0].max_corner().x() - cellGeometry[0].min_corner().x());
        cellWeights_[abacus_cell] = netlist_.pins(*cell_it).size();
        sortedCells.push_back(std::make_pair(abacus_cell, placement_.cellLocation(*cell_it)));
    }
    std::sort(sortedCells.begin(), sortedCells.end(), CellPairComparator());

    for (auto cellPair : sortedCells) {
        auto abacusCell = cellPair.first;

        double bestCost = std::numeric_limits<double>::max();
        Subrow bestSubrow;

        std::vector<RtreeNode> intersectingSubrowNodes;
        geometry::Point cellPoint(units::unit_cast<double>(cellInitialLocations_[abacusCell].x()), units::unit_cast<double>(cellInitialLocations_[abacusCell].y()));
        subrowsRtree_.query(boost::geometry::index::nearest(cellPoint, 5), std::back_inserter(intersectingSubrowNodes));
        for (auto subrowNode : intersectingSubrowNodes) {
            auto subrow = subrowNode.second;
            subrowCells_.addAssociation(subrow, abacusCell);
            placeRow(subrow);
            double cost = std::abs(units::unit_cast<double>(cellLegalLocations_[abacusCell].x()) - units::unit_cast<double>(cellInitialLocations_[abacusCell].x())) +
                    std::abs(units::unit_cast<double>(cellLegalLocations_[abacusCell].y()) - units::unit_cast<double>(cellInitialLocations_[abacusCell].y()));
            subrowCells_.eraseAssociation(subrow, abacusCell);
            if (cost < bestCost) {
                bestCost = cost;
                bestSubrow = subrow;
            }
        }

        subrowCells_.addAssociation(bestSubrow, abacusCell);
        placeRow(bestSubrow);
    }
}

void Abacus::placeRow(Subrow subrow)
{
    clusters_.clear();
    auto subrowCells = subrowCells_.parts(subrow);
    for (auto abacusCellIt = subrowCells.begin(); abacusCellIt != subrowCells.end(); ++abacusCellIt) {
        auto clusterIt = clusters_.end();
        clusterIt--;
        if (clusters_.empty() || clusterOrigins_[*clusterIt] + clusterWidths_[*clusterIt] <= cellInitialLocations_[*abacusCellIt].x()) {
            auto cluster = clusters_.add();
            clusterOrigins_[cluster] = cellInitialLocations_[*abacusCellIt].x();
            addCell(abacusCellIt, cluster);
        } else {
            addCell(abacusCellIt, *clusterIt);
            collapse(clusterIt, subrow);
        }
    }

    auto abacusCellIt = subrowCells.begin();
    for (auto cluster : clusters_) {
        ophidian::util::micrometer_t x = clusterOrigins_[cluster];
        for (; abacusCellIt != clusterLastCells_[cluster]; ++abacusCellIt) {
            cellLegalLocations_[*abacusCellIt].x(x);
            cellLegalLocations_[*abacusCellIt].y(subrowOrigins_[subrow].y());
            x = x + cellWidths_[*abacusCellIt];
        }
    }
}

void Abacus::addCell(entity_system::Association<Subrow, AbacusCell>::Parts::PartIterator abacusCellIt, Cluster cluster)
{
    auto abacusCell = *abacusCellIt;
    clusterLastCells_[cluster] = abacusCellIt;
    clusterWeights_[cluster] += cellWeights_[abacusCell];
    clusterDisplacements_[cluster] = clusterDisplacements_[cluster] + (cellWeights_[abacusCell] * (cellInitialLocations_[abacusCell].x() - clusterWidths_[cluster]));
    clusterWidths_[cluster] = clusterWidths_[cluster] + cellWidths_[abacusCell];
}

void Abacus::addCluster(Cluster cluster, Cluster nextCluster)
{
    clusterLastCells_[cluster] = clusterLastCells_[nextCluster];
    clusterWeights_[cluster] += clusterWeights_[nextCluster];
    clusterDisplacements_[cluster] = clusterDisplacements_[cluster] + clusterDisplacements_[nextCluster] - (clusterWeights_[nextCluster] * clusterWidths_[cluster]);
    clusterWidths_[cluster] = clusterWidths_[cluster] + clusterWidths_[nextCluster];
}

void Abacus::collapse(std::vector<Cluster>::const_iterator clusterIt, Subrow subrow)
{
    auto cluster = *clusterIt;

    ophidian::util::micrometer_t xMin = subrowOrigins_[subrow].x();
    ophidian::util::micrometer_t xMax = subrowUpperCorners_[subrow].x() - clusterWidths_[cluster];

    ophidian::util::micrometer_t optimalX = clusterDisplacements_[cluster] / clusterWeights_[cluster];
    optimalX = std::min(std::max(optimalX, xMin), xMax);
    clusterOrigins_[cluster] = optimalX;

    auto previousClusterIt = clusterIt;
    previousClusterIt--;
    if (clusterIt != clusters_.begin() && clusterOrigins_[*previousClusterIt] + clusterWidths_[*previousClusterIt] > clusterOrigins_[cluster]) {
        addCluster(cluster, *previousClusterIt);
        clusters_.erase(cluster);
        collapse(previousClusterIt, subrow);
    }
}

void Abacus::createSubrows()
{
    for (auto rowIt = floorplan_.rowsRange().begin(); rowIt != floorplan_.rowsRange().end(); ++rowIt) {
        auto subrow = subrows_.add();
        auto subrowOrigin = floorplan_.origin(*rowIt);
        auto rowUpperCorner = floorplan_.rowUpperRightCorner(*rowIt);
        subrowOrigins_[subrow] = subrowOrigin;
        subrowUpperCorners_[subrow] = rowUpperCorner;

        geometry::Box subrowBox(geometry::Point(units::unit_cast<double>(subrowOrigin.x()), units::unit_cast<double>(subrowOrigin.y())),
                                geometry::Point(units::unit_cast<double>(rowUpperCorner.x()), units::unit_cast<double>(rowUpperCorner.y())));
        subrowsRtree_.insert(RtreeNode(subrowBox, subrow));
    }

    for (auto cellIt = netlist_.begin(circuit::Cell()); cellIt != netlist_.end(circuit::Cell()); ++cellIt) {
        if (placement_.isFixed(*cellIt)) {
            auto cellGeometry = placementMapping_.geometry(*cellIt);
            for (auto cellBox : cellGeometry) {
                std::vector<RtreeNode> intersectingSubrowNodes;
                subrowsRtree_.query(boost::geometry::index::intersects(cellBox), std::back_inserter(intersectingSubrowNodes));
                for (auto subrowNode : intersectingSubrowNodes) {
                    auto subrow = subrowNode.second;

                    auto leftSubrow = subrows_.add();
                    subrowOrigins_[leftSubrow] = subrowOrigins_[subrow];
                    subrowUpperCorners_[leftSubrow] = util::Location(cellBox.min_corner().x(), cellBox.min_corner().x());
                    geometry::Box leftSubrowBox(geometry::Point(units::unit_cast<double>(subrowOrigins_[leftSubrow].x()), units::unit_cast<double>(subrowOrigins_[leftSubrow].y())),
                                            geometry::Point(units::unit_cast<double>(subrowUpperCorners_[leftSubrow].x()), units::unit_cast<double>(subrowUpperCorners_[leftSubrow].y())));

                    auto rightSubrow = subrows_.add();
                    subrowOrigins_[rightSubrow] = util::Location(cellBox.max_corner().x(), cellBox.max_corner().x());
                    subrowUpperCorners_[rightSubrow] = subrowUpperCorners_[subrow];
                    geometry::Box rightSubrowBox(geometry::Point(units::unit_cast<double>(subrowOrigins_[rightSubrow].x()), units::unit_cast<double>(subrowOrigins_[rightSubrow].y())),
                                            geometry::Point(units::unit_cast<double>(subrowUpperCorners_[rightSubrow].x()), units::unit_cast<double>(subrowUpperCorners_[rightSubrow].y())));

                    subrowsRtree_.remove(subrowNode);
                    subrowsRtree_.insert(RtreeNode(leftSubrowBox, leftSubrow));
                    subrowsRtree_.insert(RtreeNode(rightSubrowBox, rightSubrow));
                }
            }
        }
    }
}

}
}
