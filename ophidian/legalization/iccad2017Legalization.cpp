#include "iccad2017Legalization.h"

#include <boost/lexical_cast.hpp>

namespace ophidian
{
namespace legalization
{
iccad2017Legalization::iccad2017Legalization(design::Design &design)
    : mDesign(design),
      mCellsInitialLocations(design.netlist().makeProperty<util::Location>(circuit::Cell())),
      mMultirowAbacus(design.netlist(), design.floorplan(), design.placement(), design.placementMapping()),
      mFenceRegionIsolation(design)
{
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt) {
        mCellsInitialLocations[*cellIt] = mDesign.placement().cellLocation(*cellIt);
    }
}

void iccad2017Legalization::flipCells()
{
    auto siteHeight = mDesign.floorplan().siteUpperRightCorner(*mDesign.floorplan().sitesRange().begin()).y();
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
    {
        if (!mDesign.placement().isFixed(*cellIt)) {
            auto cellGeometry = mDesign.placementMapping().geometry(*cellIt);
            auto cellHeight = ophidian::util::micrometer_t(cellGeometry[0].max_corner().y() - cellGeometry[0].min_corner().y());
            if(std::fmod((cellHeight/siteHeight), 2.0))
            {
                //Odd-sized cells
                auto cellPosition = mDesign.placement().cellLocation(*cellIt).y();
                if(std::fmod((cellPosition/siteHeight), 2.0))
                {
                    //placed in odd line -> flip cell
                    mDesign.placement().cellOrientation(*cellIt, "S");
                }
            }
        }
    }
}

void iccad2017Legalization::legalize()
{
    //posiciona fences (paralelo)
    for(auto fenceIt = mDesign.fences().range().begin(); fenceIt < mDesign.fences().range().end(); fenceIt++)
    {
        auto fence = *fenceIt;
        std::vector<circuit::Cell> cells (mDesign.fences().members(fence).begin(), mDesign.fences().members(fence).end());
        mMultirowAbacus.legalizePlacement(cells, mDesign.fences().area(fence));
    }

    mFenceRegionIsolation.isolateAllFenceCells();

    //posiciona circuito
    std::vector<circuit::Cell> cells;
    cells.reserve(mDesign.netlist().size(circuit::Cell()));
    for(auto cellIt = mDesign.netlist().begin(circuit::Cell()); cellIt != mDesign.netlist().end(circuit::Cell()); ++cellIt)
    {
        if(!mDesign.placement().cellHasFence(*cellIt))
        {
            cells.push_back(*cellIt);
        }
    }
    geometry::Box chipArea(mDesign.floorplan().chipOrigin().toPoint(), mDesign.floorplan().chipUpperRightCorner().toPoint());
    util::MultiBox legalizationArea({chipArea});
    mMultirowAbacus.legalizePlacement(cells, legalizationArea);


    mFenceRegionIsolation.restoreAllFenceCells();

    //flip cells
    flipCells();
}

} // namespace legalization
} // namespace ophidian
