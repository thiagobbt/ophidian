#include "AbacusPlaceRow.h"

namespace ophidian {
namespace legalization {
AbacusPlaceRow::AbacusPlaceRow(Subrows &subrows, entity_system::Property<AbacusCell, util::Location> &cellInitialLocations, entity_system::Property<AbacusCell, util::Location> &cellLegalLocations, entity_system::Property<AbacusCell, util::micrometer_t> &cellWidths, entity_system::Property<AbacusCell, double> &cellWeigths)
    : subrows_(subrows), cellInitialLocations_(cellInitialLocations), cellLegalLocations_(cellLegalLocations), cellWidths_(cellWidths), cellWeights_(cellWeigths),
      clusterOrigins_(clusters_), clusterWidths_(clusters_), clusterWeights_(clusters_), clusterDisplacements_(clusters_), clusterFirstCells_(clusters_), clusterLastCells_(clusters_) {

}

void AbacusPlaceRow::operator ()(Subrow subrow, const std::vector<AbacusCell> & subrowCells, util::micrometer_t siteWidth) {
    clusters_.clear();
    for (auto abacusCellIt = subrowCells.begin(); abacusCellIt != subrowCells.end(); ++abacusCellIt) {        
        auto clusterIt = clusters_.end();
        clusterIt--;
        ophidian::util::micrometer_t xMin = subrows_.origin(subrow).x();
        ophidian::util::micrometer_t xMax = subrows_.upperCorner(subrow).x() - cellWidths_[*abacusCellIt];
        ophidian::util::micrometer_t x = cellInitialLocations_[*abacusCellIt].x();
        x = std::min(std::max(x, xMin), xMax);
        x = std::floor(units::unit_cast<double>(x / siteWidth)) * siteWidth;
        if (clusters_.empty() || clusterOrigins_[*clusterIt] + clusterWidths_[*clusterIt] <= x) {
            auto cluster = clusters_.add();

            clusterOrigins_[cluster] = x;
            addCell(abacusCellIt, cluster);

            clusterFirstCells_[cluster] = abacusCellIt;
        } else {
            addCell(abacusCellIt, *clusterIt);
            collapse(clusterIt, subrow, siteWidth);
        }
    }

    auto abacusCellIt = subrowCells.begin();
    for (auto cluster : clusters_) {
        ophidian::util::micrometer_t x = clusterOrigins_[cluster];
        for (; abacusCellIt != clusterLastCells_[cluster]; ++abacusCellIt) {
            cellLegalLocations_[*abacusCellIt].x(x);
            cellLegalLocations_[*abacusCellIt].y(subrows_.origin(subrow).y());
            x = x + cellWidths_[*abacusCellIt];
        }
    }
}

void AbacusPlaceRow::addCell(std::vector<AbacusCell>::const_iterator abacusCellIt, Cluster cluster)
{
    auto abacusCell = *abacusCellIt;
    auto lastCellIt = abacusCellIt;
    lastCellIt++;
    clusterLastCells_[cluster] = lastCellIt;
    clusterWeights_[cluster] += cellWeights_[abacusCell];
    clusterDisplacements_[cluster] = clusterDisplacements_[cluster] + (cellWeights_[abacusCell] * (cellInitialLocations_[abacusCell].x() - clusterWidths_[cluster]));
    clusterWidths_[cluster] = clusterWidths_[cluster] + cellWidths_[abacusCell];
}

void AbacusPlaceRow::addCluster(Cluster cluster, Cluster nextCluster)
{
    auto currentClusterWeight = clusterWeights_[cluster];
    auto nextClusterWeight = clusterWeights_[nextCluster];
    clusterLastCells_[cluster] = clusterLastCells_[nextCluster];
    clusterWeights_[cluster] += clusterWeights_[nextCluster];
    clusterDisplacements_[cluster] = clusterDisplacements_[cluster] + clusterDisplacements_[nextCluster] - (clusterWeights_[nextCluster] * clusterWidths_[cluster]);
    clusterWidths_[cluster] = clusterWidths_[cluster] + clusterWidths_[nextCluster];
}

void AbacusPlaceRow::collapse(std::vector<Cluster>::const_iterator clusterIt, Subrow subrow, util::micrometer_t siteWidth)
{
    auto cluster = *clusterIt;

    ophidian::util::micrometer_t xMin = subrows_.origin(subrow).x();
    ophidian::util::micrometer_t xMax = subrows_.upperCorner(subrow).x() - clusterWidths_[cluster];

    ophidian::util::micrometer_t optimalX = clusterDisplacements_[cluster] / clusterWeights_[cluster];

//    std::vector<util::micrometer_t> displacements;
//    util::micrometer_t accumulatedWidth(0);
//    for (auto cellIt = clusterFirstCells_[cluster]; cellIt != clusterLastCells_[cluster]; cellIt++) {
//        util::micrometer_t displacement = cellInitialLocations_[*cellIt].x() - accumulatedWidth;
//        displacements.push_back(displacement);
//        accumulatedWidth = accumulatedWidth + cellWidths_[*cellIt];
//    }
//    std::sort(displacements.begin(), displacements.end());
//    auto medianIndex = (displacements.size() > 2) ? (displacements.size() / 2) + 1 : 1;
//    ophidian::util::micrometer_t optimalX = displacements[medianIndex];

    optimalX = std::min(std::max(optimalX, xMin), xMax);
    optimalX = std::floor(units::unit_cast<double>(optimalX / siteWidth)) * siteWidth;
    clusterOrigins_[cluster] = optimalX;

    auto previousClusterIt = clusterIt;
    previousClusterIt--;
    if (clusterIt != clusters_.begin() && clusterOrigins_[*previousClusterIt] + clusterWidths_[*previousClusterIt] > clusterOrigins_[cluster]) {
        addCluster(*previousClusterIt, cluster);
        clusters_.erase(cluster);
        collapse(previousClusterIt, subrow, siteWidth);
    }
}
}
}
