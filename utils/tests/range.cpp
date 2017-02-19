#include <map>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "utils/range.h"
#include "utils/types.h"

namespace meta::utils {
namespace {

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
