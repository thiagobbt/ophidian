#include "AbacusPlaceRow.h"

namespace ophidian {
namespace legalization {
AbacusPlaceRow::AbacusPlaceRow(Subrows &subrows, entity_system::Property<AbacusCell, util::Location> &cellInitialLocations, entity_system::Property<AbacusCell, util::Location> &cellLegalLocations, entity_system::Property<AbacusCell, util::micrometer_t> &cellWidths, entity_system::Property<AbacusCell, double> &cellWeigths)
    : subrows_(subrows), cellInitialLocations_(cellInitialLocations), cellLegalLocations_(cellLegalLocations), cellWidths_(cellWidths), cellWeights_(cellWeigths),
      clusterOrigins_(clusters_), clusterWidths_(clusters_), clusterWeights_(clusters_), clusterDisplacements_(clusters_), clusterLastCells_(clusters_) {

}

void AbacusPlaceRow::operator ()(Subrow subrow, const std::vector<AbacusCell> & subrowCells) {
    clusters_.clear();
    for (auto abacusCellIt = subrowCells.begin(); abacusCellIt != subrowCells.end(); ++abacusCellIt) {
        auto clusterIt = clusters_.end();
        clusterIt--;
        if (clusters_.empty() || clusterOrigins_[*clusterIt] + clusterWidths_[*clusterIt] <= cellInitialLocations_[*abacusCellIt].x()) {
            auto cluster = clusters_.add();

            ophidian::util::micrometer_t xMin = subrows_.origin(subrow).x();
            ophidian::util::micrometer_t xMax = subrows_.upperCorner(subrow).x() - cellWidths_[*abacusCellIt];
            ophidian::util::micrometer_t x = cellInitialLocations_[*abacusCellIt].x();
            x = std::min(std::max(x, xMin), xMax);

            clusterOrigins_[cluster] = x;
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
    clusterLastCells_[cluster] = clusterLastCells_[nextCluster];
    clusterWeights_[cluster] += clusterWeights_[nextCluster];
    clusterDisplacements_[cluster] = clusterDisplacements_[cluster] + clusterDisplacements_[nextCluster] - (clusterWeights_[nextCluster] * clusterWidths_[cluster]);
    clusterWidths_[cluster] = clusterWidths_[cluster] + clusterWidths_[nextCluster];
}

void AbacusPlaceRow::collapse(std::vector<Cluster>::const_iterator clusterIt, Subrow subrow)
{
    auto cluster = *clusterIt;

    ophidian::util::micrometer_t xMin = subrows_.origin(subrow).x();
    ophidian::util::micrometer_t xMax = subrows_.upperCorner(subrow).x() - clusterWidths_[cluster];

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
}
}
