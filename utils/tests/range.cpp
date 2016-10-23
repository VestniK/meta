#include <map>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "utils/range.h"
#include "utils/types.h"

namespace meta::utils {
namespace {

struct TestData {
    std::vector<int> given;
    std::vector<std::tuple<int, int>> expected;
};

class Ranges: public ::testing::TestWithParam<TestData> {};

TEST_P(Ranges, segments) {
    auto param = GetParam();
    std::vector<std::tuple<int, int>> segs;
    for (auto seg: segments(param.given))
        segs.emplace_back(seg);
    EXPECT_EQ(param.expected, segs);
}

INSTANTIATE_TEST_CASE_P(SegmentsRange, Ranges, ::testing::Values(
    TestData{{}, {}},
    TestData{{1}, {}},
    TestData{{1,2}, {{1,2}}},
    TestData{{1,2,3}, {{1,2}, {2,3}}},
    TestData{{1, 2, 3, 4, 5}, {{1,2}, {2,3}, {3,4}, {4,5}}}
));

struct Point {
    int x;
    int y;
};

TEST(Ranges, byMember) {
    std::vector<Point> arr = {{10, 10}, {0, 10}, {0, 0}, {10, 0}};
    size_t pos = 0;
    for (int ptx: arr | &Point::x) {
        ASSERT_LT(pos, std::size(arr));
        EXPECT_EQ(ptx, arr[pos++].x);
    }
    EXPECT_EQ(pos, std::size(arr));

    pos = 0;
    for (int pty: arr | &Point::y) {
        ASSERT_LT(pos, std::size(arr));
        EXPECT_EQ(pty, arr[pos++].y);
    }
    EXPECT_EQ(pos, std::size(arr));
}

TEST(Range, byMemberOfMember) {
    std::map<std::string, Point> dict = {
        {"qwe"s, {0, 5}},
        {"asd"s, {4, 5}},
        {"foo"s, {3, 2}}
    };
    auto it = dict.begin();
    for (int x: dict | &std::pair<const std::string, Point>::second | &Point::x) {
        ASSERT_NE(it, dict.end());
        EXPECT_EQ(x, it->second.x);
        ++it;
    }
    ASSERT_EQ(it, dict.end());
}

}
} // namespace meta::utils
