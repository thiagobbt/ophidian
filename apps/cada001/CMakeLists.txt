set(SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        )

INCLUDE_DIRECTORIES( ${THIRD_PARTY_PATH} )
LINK_DIRECTORIES( ${THIRD_PARTY_PATH}/DEF/lib/ ${THIRD_PARTY_PATH}/LEF/lib/ )

add_subdirectory(wrapper)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBRARIES OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable( cada001 ${SOURCE} ${HEADERS} )
target_link_libraries(cada001 ophidian_design ophidian_circuit ophidian_geometry ophidian_parser ophidian_interconnection ophidian_floorplan ophidian_placement ophidian_standard_cell ophidian_timing ophidian_legalization emon ophidian_entitysystem)

include_directories(${CMAKE_SOURCE_DIR})

