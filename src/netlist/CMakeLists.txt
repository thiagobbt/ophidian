find_package( Boost 1.59 )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
add_library (netlist cells.cpp pins.cpp nets.cpp netlist.cpp verilog2netlist.cpp )
target_include_directories ( netlist PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries( netlist ${Boost_LIBRARIES} standard_cell )
