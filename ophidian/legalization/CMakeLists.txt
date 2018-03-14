INCLUDE_DIRECTORIES( ${THIRD_PARTY_PATH} ${THIRD_PARTY_PATH}/gurobi702/linux64/include)

file(GLOB ophidian_legalization_SRC
    "*.h"
    "*.cpp"
)
add_library(ophidian_legalization ${ophidian_legalization_SRC})
target_link_libraries(ophidian_legalization PUBLIC ophidian_util)
install(TARGETS ophidian_legalization DESTINATION lib)
install(FILES Abacus.h DESTINATION include/ophidian/legalization)
