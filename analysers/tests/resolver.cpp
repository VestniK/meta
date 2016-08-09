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

namespace meta::analysers {
namespace {

class ResolveErrors: public utils::ErrorTest {};

TEST_P(ResolveErrors, resolveErrors) {
    auto param = GetParam();
    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, "test.meta", param.input);
    auto ast = parser.ast();
    try {
        v2::resolve(ast, act.dictionary());
        FAIL() << "Error was not detected: " << param.errMsg;
    } catch (const SemanticError& err) {
        EXPECT_EQ(err.what(), param.errMsg) << err.what();
    }
}

INSTANTIATE_TEST_CASE_P(Resolver, ResolveErrors, ::testing::Values(
    utils::ErrorTestData{
        .input=R"META(
            package test;

            extern int foo(int x) {return 2*x;}
        )META",
        .errMsg=R"(Extern function 'foo' must not have implementation)"
    },
    utils::ErrorTestData{
        .input=R"META(
            package test;

            export string foo(int x);
        )META",
        .errMsg=R"(Implementation missing for the function 'foo')"
    },
    utils::ErrorTestData{
        .input=R"META(
            package test;

            public string foo(int x);
        )META",
        .errMsg=R"(Implementation missing for the function 'foo')"
    },
    utils::ErrorTestData{
        .input=R"META(
            package test;

            protected string foo(int x);
        )META",
        .errMsg=R"(Implementation missing for the function 'foo')"
    },
    utils::ErrorTestData{
        .input=R"META(
            package test;

            private string foo(int x);
        )META",
        .errMsg=R"(Implementation missing for the function 'foo')"
    },
    utils::ErrorTestData{
        .input=R"META(
            package test;

            int foo(int x = 42, int y) {return x + y;}
        )META",
        .errMsg=R"(Argument 'y' has no default value while previous argument 'x' has)"
    },
    utils::ErrorTestData{
        .input=R"META(
            package test;

            int foo(int x = 42, int y, int z)  {return x + y + z;}
        )META",
        .errMsg=R"(Argument 'y' has no default value while previous argument 'x' has)"
    },
    utils::ErrorTestData{
        .input=R"META(
            package test;

            int foo(int x, int y = 42, int z)  {return x + y + z;}
        )META",
        .errMsg=R"(Argument 'z' has no default value while previous argument 'y' has)"
    }
));

} // anonymous namespace
} // namespace meta::analysers

using namespace meta;
using namespace meta::analysers;

using namespace std::literals;

namespace {

class Resolver: public testing::TestWithParam<std::string> {};

TEST_P(Resolver, resolveErrors) {
    const auto lib = R"META(
        package some.lib;

        export int foo1(int x) {return x/2;}

        public int foo2(int x) {return 2*x;}

        protected int foo3(int x) {return x*x;}

        private int foo4(int x) {return x*x - 2*x;}
    )META"s;
    const std::string& input = GetParam();
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("lib.meta", lib));
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    try {
        resolve(ast, act.dictionary());
        ASSERT_TRUE(false) << "Input code contains symbol resolution errors which were not found";
    } catch (SemanticError &err) {
        ASSERT_EQ(err.tokens().linenum(), 2) << err.what() << ": " << utils::string_view(err.tokens());
        ASSERT_EQ(err.tokens().colnum(), 1) << err.what() << ": " << utils::string_view(err.tokens());
    }
}

INSTANTIATE_TEST_CASE_P(semanticErrors, Resolver, ::testing::Values(
    "package test; auto foo(int x) {\nbool x; return x;}", // argument redifinition
    "package test; auto foo() {int x = 0; \nbool x; return x;}", // shadow another var
    "package test; auto foo(int x) {\nbool b; return x;}" // never used
));

}
