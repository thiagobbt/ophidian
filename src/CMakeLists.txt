add_subdirectory (parsing)
add_subdirectory (interconnection)
add_subdirectory (standard_cell)
add_subdirectory (entity_system)
add_subdirectory (netlist)
add_subdirectory (placement)
add_subdirectory (legalization)
add_subdirectory (floorplan)
add_subdirectory (timing)
add_subdirectory (timing-driven_placement)
add_subdirectory (density)
add_subdirectory (routing)
add_subdirectory (clock_tree_synthesis)
add_subdirectory (register_clustering)

if(BUILD_GUI)
    add_subdirectory (gui)
endif(BUILD_GUI)
