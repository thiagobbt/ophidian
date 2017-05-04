file(GLOB ophidian_legalization_SRC
    "*.h"
    "*.cpp"
)
add_library(ophidian_legalization ${ophidian_legalization_SRC})
install(TARGETS ophidian_legalization DESTINATION lib)
install(FILES Abacus.h DESTINATION include/ophidian/legalization)
