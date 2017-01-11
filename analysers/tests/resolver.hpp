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

#include <memory>

#include <gtest/gtest.h>

#include "utils/testtools.h"

#include "parser/metaparser.h"

#include "analysers/actions.h"
#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace meta::analysers::tests::resolver {
namespace {

class ResolveErrors: public utils::ErrorTest {};

TEST_P(ResolveErrors, resolveErrors) {
    const auto& param = GetParam();
    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, param.input);
    auto ast = parser.ast();
    try {
        resolve(ast, act.dictionary());
        FAIL() << "Error was not detected: " << param.errMsg;
    } catch (const SemanticError& err) {
        EXPECT_EQ(param.errMsg, err.what()) << err.what();
    }
}

utils::ErrorTestData testData[] = {
    {
        .input = R"META(
            package test;

            extern int foo(int x) {return 2*x;}
        )META"_fake_src,
        .errMsg = "Extern function 'foo' must not have implementation"
    },
    {
        .input=R"META(
            package test;

            export string foo(int x);
        )META"_fake_src,
        .errMsg = "Implementation missing for the function 'foo'"
    },
    {
        .input = R"META(
            package test;

            public string foo(int x);
        )META"_fake_src,
        .errMsg = "Implementation missing for the function 'foo'"
    },
    {
        .input=R"META(
            package test;

            protected string foo(int x);
        )META"_fake_src,
        .errMsg = "Implementation missing for the function 'foo'"
    },
    {
        .input=R"META(
            package test;

            private string foo(int x);
        )META"_fake_src,
        .errMsg = "Implementation missing for the function 'foo'"
    },
    {
        .input = R"META(
            package test;

            int foo(int x = 42, int y) {return x + y;}
        )META"_fake_src,
        .errMsg = "Argument 'y' has no default value while previous argument 'x' has"
    },
    {
        .input = R"META(
            package test;

            int foo(int x = 42, int y, int z)  {return x + y + z;}
        )META"_fake_src,
        .errMsg = "Argument 'y' has no default value while previous argument 'x' has"
    },
    {
        .input = R"META(
            package test;

            int foo(int x, int y = 42, int z)  {return x + y + z;}
        )META"_fake_src,
        .errMsg = "Argument 'z' has no default value while previous argument 'y' has"
    },
    {
        .input = R"META(
            package test;

            bool foo(int x, int y) {bool x = y < 0; return x;}
        )META"_fake_src,
        .errMsg =
            "Variable 'bool x' conflicts with other declarations.\n"
            "test.meta:4:22: notice: Variable 'int x' (function argument)"
    },
    {
        .input = R"META(
            package test;

            bool foo(int x, int y) {
                if (x < 0) {
                    bool x = y < 0;
                    return x;
                }
                return true;
            }
        )META"_fake_src,
        .errMsg =
            "Variable 'bool x' conflicts with other declarations.\n"
            "test.meta:4:22: notice: Variable 'int x' (function argument)"
    },
    {
        .input=R"META(
            package test;

            int foo(int x) {
                bool var = x < 0;
                if (var)
                    return -x;
                int var = x - 9;
                return var;
            }
        )META"_fake_src,
        .errMsg=
            "Variable 'int var' conflicts with other declarations.\n"
            "test.meta:5:17: notice: Variable 'bool var'"
    },
    {
        .input=R"META(
            package test;

            int foo(int x) {
                return y;
            }
        )META"_fake_src,
        .errMsg = "Undefined variable 'y'"
    },
    {
        .input=R"META(
            package test;

            int foo(int x) {
                return 5;
            }
        )META"_fake_src,
        .errMsg = "Variable 'x' is never used"
    },
    {
        .input=R"META(
            package test;

            int foo(int x) {
                bool b;
                return x;
            }
        )META"_fake_src,
        .errMsg = "Variable 'b' is never used"
    },
    {
        .input = R"META(
            package test;

            int foo(int x) {
                int y;
                return x+y;
            }
        )META"_fake_src,
        .errMsg = "Variable 'y' accessed before initialization"
    }
};
INSTANTIATE_TEST_CASE_P(Resolver, ResolveErrors, ::testing::ValuesIn(testData));

} // anonymous namespace
} // namespace meta::analysers
