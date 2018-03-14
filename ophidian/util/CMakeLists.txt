file(GLOB ophidian_util_SRC
    "*.h"
    "*.cpp"
)
add_library(ophidian_util ${ophidian_util_SRC})
target_include_directories(ophidian_util PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(ophidian_util PUBLIC )
install(TARGETS ophidian_util DESTINATION lib)
