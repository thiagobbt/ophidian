#ifndef PLACEMENTMAPPING_H
#define PLACEMENTMAPPING_H

#include <ophidian/placement/Placement.h>
#include <ophidian/placement/Library.h>
#include <ophidian/circuit/LibraryMapping.h>
#include <ophidian/geometry/Operations.h>

namespace ophidian
{
namespace placement
{
class PlacementMapping
{
public:
    //! Placement mapping Constructor
    /*!
       \brief Constructs an empty placement mapping from placement and circuit information.
       \param placement Circuit placement
       \param library Placement library
       \param netlist Circuit netlist.
       \param libraryMapping library mapping between the netlist and standard cells library
     */
    PlacementMapping(const Placement & placement, const Library & library, const circuit::Netlist & netlist, const circuit::LibraryMapping & libraryMapping);

    //! Cell geometry getter
    /*!
       \brief Get the geometry of a cell in the circuit.
       \param cell Cell entity to get the geometry.
       \return Geometry of the cell.
     */
    util::MultiBox geometry(const circuit::Cell & cell) const;

    //! Pin location getter
    /*!
       \brief Get the location of a pin in the circuit.
       \param cell Pin entity to get the location.
       \return Location of the cell.
     */
    util::Location location(const circuit::Pin & pin) const;

    //! Cell Aligmnment getter
    /*!
       \brief Gets the power aligment of a cell.
       \param cell Cell entity to get the power aligment .
       \return RowAlignment of the cell.
     */
    placement::RowAlignment alignment(const circuit::Cell & cell) const;

private:
    const Placement & placement_;
    const Library & library_;
    const circuit::Netlist & netlist_;
    const circuit::LibraryMapping & libraryMapping_;
};
} // namespace placement
} // namespace ophidian

#endif // PLACEMENTMAPPING_H
