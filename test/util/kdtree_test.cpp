#include <catch.hpp>
#include <ophidian/util/KDtree.h>

using namespace ophidian::util;
using Range = ophidian::geometry::Box;
using Point = ophidian::geometry::Point;

TEST_CASE("KDtree range search Test: ", "[kdtree][units]")
{
    std::vector<Point> points = {{1, 1.5}, {1.5, 3.5}, {2.5, 4.5}, {3, 2}, {3.5, 1.5}, {4.5, 2.5}, {5, 4.5}};
    KDtree<Point> tree;
    tree.reserve(points.size());
    for (size_t i = 0; i < points.size(); i++) {
        tree.add(points[i], std::make_shared<Point>(points[i]));
    }
    tree.build(Range(Point(0,0), Point(6, 6)));
    auto result = tree.range_search(Range(Point(0.5, 0.5), Point(5.5, 5.5)));
    REQUIRE(result.size() == 7);
    result = tree.range_search(Range(Point(0.5, 1), Point(2, 4)));
    REQUIRE(result.size() == 2);
    result = tree.range_search(Range(Point(2.5, 1), Point(5, 3)));
    REQUIRE(result.size() == 3);
    result = tree.range_search(Range(Point(3, 1), Point(4.5, 2.5)));
    REQUIRE(result.size() == 1);
    result = tree.range_search(Range(Point(1, 1), Point(2, 2)));
    REQUIRE(result.size() == 0);
    result = tree.range_search(Range(Point(0, 0), Point(2, 4)));
}
