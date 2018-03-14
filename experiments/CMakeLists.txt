set(SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        )

#add_library(Catch INTERFACE)
#target_include_directories(Catch INTERFACE .)

INCLUDE_DIRECTORIES( ${THIRD_PARTY_PATH} /opt/gurobi702/linux64/include)
LINK_DIRECTORIES( ${THIRD_PARTY_PATH}/DEF/lib/ ${THIRD_PARTY_PATH}/LEF/lib/ /opt/gurobi702/linux64/lib)

add_subdirectory(iccad2017)

#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#SET(BUILD_SHARED_LIBRARIES OFF)
#SET(CMAKE_EXE_LINKER_FLAGS "-static")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_executable( run_experiments ${SOURCE} ${HEADERS} )
target_link_libraries(run_experiments Catch ophidian_util ophidian_design ophidian_circuit ophidian_geometry ophidian_parser ophidian_interconnection ophidian_floorplan ophidian_placement ophidian_standard_cell ophidian_timing ophidian_legalization emon ophidian_entitysystem gurobi_g++5.2 gurobi70)

include_directories(${CMAKE_SOURCE_DIR})
add_test(NAME unit_test COMMAND run_experiments "~[Profiling] ~[superblue18obs]")

add_custom_command(
        TARGET run_experiments POST_BUILD
        COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/input_files ${CMAKE_CURRENT_BINARY_DIR}/input_files
        )

message("Creating symlink for test files")
#execute_process(COMMAND ln -f -s ${CMAKE_SOURCE_DIR}/experiments/input_files ${CMAKE_BINARY_DIR}/experiments/.
 #               RESULT_VARIABLE result )
