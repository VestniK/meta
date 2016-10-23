/*
 * Meta language compiler
 * Copyright (C) 2014  Sergey Vidyuk <sir.vestnik@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gtest/gtest.h>

#include "utils/testtools.h"

#include "parser/metaparser.h"

#include "analysers/actions.h"
#include "analysers/reachabilitychecker.h"
#include "analysers/semanticerror.h"

namespace meta::analysers::tests {
namespace {

class Reachability: public utils::ErrorTest {};

TEST_P(Reachability, resolveErrors) {
    const auto& param = GetParam();
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, param.input);
    auto ast = parser.ast();
    try {
        checkReachability(ast);
        FAIL() << "Error was not detected: " << param.errMsg;
    } catch (const SemanticError& err) {
        EXPECT_EQ(param.errMsg, err.what()) << err.what();
    }
}

utils::ErrorTestData testData[] = {
    {
        .input = R"META(
            package test;

            auto foo(int x) {
                return x;
                bool y = x > 0;
            }
        )META"_fake_src,
        .errMsg = "Code is unreachable due to return statement at position 5:17"
    },
    {
        .input = R"META(
            package test;

            auto foo(int x) {
                int y = 2*x;
                {
                    y = y + 5;
                    return y;
                }
                return 0;
            }
        )META"_fake_src,
        .errMsg = "Code is unreachable due to return statement at position 8:21"
    },
    {
        .input = R"META(
            package test;

            auto foo(int x) {
                if (x < 0)
                    return -x;
                else {
                    int y = 5 + x;
                    return y;
                    y = y + x;
                }
                return 0;
            }
        )META"_fake_src,
        .errMsg = "Code is unreachable due to return statement at position 9:21"
    }
};
INSTANTIATE_TEST_CASE_P(semanticErrors, Reachability, ::testing::ValuesIn(testData));

} // anonymous namespace
} // namespace meta::analysers::tests
