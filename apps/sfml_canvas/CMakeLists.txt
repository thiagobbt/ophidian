set(SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        )

find_package(SFML 2 REQUIRED system window graphics network audio)

INCLUDE_DIRECTORIES(${SFML_INCLUDE_DIR} ${THIRD_PARTY_PATH} )
LINK_DIRECTORIES( ${THIRD_PARTY_PATH}/DEF/lib/ ${THIRD_PARTY_PATH}/LEF/lib/)

#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#SET(BUILD_SHARED_LIBRARIES OFF)
#SET(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable( sfml_canvas ${SOURCE} ${HEADERS} )
target_link_libraries(sfml_canvas ophidian_design ophidian_circuit ophidian_geometry ophidian_parser ophidian_interconnection ophidian_floorplan ophidian_placement ophidian_standard_cell ophidian_timing ophidian_legalization emon ophidian_entitysystem ${SFML_LIBRARIES})

include_directories(${CMAKE_SOURCE_DIR})
