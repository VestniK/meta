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

#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

#include "parser/actions.h"
#include "parser/metaparser.h"

using namespace meta;
using namespace meta::analysers;

namespace {

class Resolver: public testing::TestWithParam<const char *>
{
public:
};

}

TEST_P(Resolver, resolveErrors) {
    const char *lib = R"META(
        package some.lib;

        export int foo1(int x) {return x/2;}

        public int foo2(int x) {return 2*x;}

        protected int foo3(int x) {return x*x;}

        private int foo4(int x) {return x*x - 2*x;}
    )META";
    const char *input = GetParam();
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    parser.setSourcePath("lib.meta");
    ASSERT_NO_THROW(parser.parse(lib, strlen(lib)));
    parser.setSourcePath("test.meta");
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    try {
        resolve(ast, act.dictionary());
        ASSERT_TRUE(false) << "Input code contains symbol resolvation errors which were not found";
    } catch (SemanticError &err) {
        ASSERT_EQ(err.tokens().linenum(), 2) << err.what() << ": " << utils::string_view(err.tokens());
        ASSERT_EQ(err.tokens().colnum(), 1) << err.what() << ": " << utils::string_view(err.tokens());
    }
}

INSTANTIATE_TEST_CASE_P(semanticErrors, Resolver, ::testing::Values(
    "package test; auto foo(int x) {\nbool x; return x;}", // argument redifinition
    "package test; auto foo() {int x = 0; \nbool x; return x;}", // shadow another var
    "package test; auto foo(int x) {\nbool b; return x;}", // never used
    "package test; import some.lib.foo1; import some.lib.foo2; \nimport some.lib.foo3; int foo(int x) {return foo1(x) + foo2(x) + foo3(x);}", // import ptotected func
    "package some.lib.subpkg; import some.lib.foo1; import some.lib.foo2; import some.lib.foo3; \nimport some.lib.foo4; int foo() {return foo1(x) + foo2(x) + foo3(x) + foo4(x);}", // import private func
    "package test; \nextern int foo() {return 5;}", // extern with body
    "package test; \nexport int foo();", // local function without body
    "package test; \npublic int foo();", // local function without body
    "package test; \nprotected int foo();", // local function without body
    "package test; \nprivate int foo();" // local function without body
));
