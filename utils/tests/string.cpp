#include <gtest/gtest.h>

#include "utils/string.h"

namespace meta::utils {
namespace {

struct TestData {
    string_view str;
    std::vector<string_view> expected;
};
class SplitTest: public ::testing::TestWithParam<TestData> {};
TestData testData[] = {
    {"", {}},
    {"qwe", {"qwe"}},
    {"qwe,rty", {"qwe", "rty"}},
    {"qwe,asd,rty", {"qwe", "asd", "rty"}},
    {",asd,rty", {"", "asd", "rty"}},
    {"qwe,,rty", {"qwe", "", "rty"}},
    {"qwe,asd,", {"qwe", "asd", ""}},
    {"qwe,,,,asd,rty", {"qwe", "", "", "", "asd", "rty"}}
};
INSTANTIATE_TEST_CASE_P(DifferentSplits, SplitTest, ::testing::ValuesIn(testData));

TEST_P(SplitTest, splitStrings) {
    auto data = GetParam();
    auto rng = split(data.str, ',');
    EXPECT_EQ(rng.empty(), data.str.empty());
    size_t pos = 0;
    for (string_view item: rng) {
        ASSERT_LT(pos, data.expected.size());
        EXPECT_EQ(item, data.expected[pos]);
        ++pos;
    }
    EXPECT_EQ(pos, data.expected.size());
}

} // anonymous namespace
} // namespace meta::utils
