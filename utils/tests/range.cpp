#include <map>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "utils/range.h"

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

}
} // namespace meta::utils
