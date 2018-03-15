set(SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# INCLUDE_DIRECTORIES(${SFML_INCLUDE_DIR} ${THIRD_PARTY_PATH} )
LINK_DIRECTORIES( ${THIRD_PARTY_PATH}/DEF/lib/ ${THIRD_PARTY_PATH}/LEF/lib/)

#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#SET(BUILD_SHARED_LIBRARIES OFF)
#SET(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(cell_legalization
	${SOURCE}
	${HEADERS}
)

target_link_libraries(cell_legalization
	ophidian_design
	ophidian_parser
	ophidian_circuit
	ophidian_geometry
	# ophidian_interconnection
	ophidian_floorplan
	ophidian_placement
	ophidian_standard_cell
	# ophidian_timing
	ophidian_legalization
	emon
	ophidian_entitysystem
    ophidian_util
)

include_directories(${CMAKE_SOURCE_DIR})
