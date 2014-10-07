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

#include "parser/metaparser.h"
#include "parser/parse.h"

namespace {

class Resolver: public testing::TestWithParam<const char *>
{
public:
};

}

TEST_P(Resolver, resolveErrors) {
    const char *input = GetParam();
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    try {
        analysers::resolve(ast.get());
        ASSERT_TRUE(false) << "Input code contains symbol resolvation errors which were not found";
    } catch (analysers::SemanticError &err) {
        ASSERT_EQ(err.tokens().linenum(), 2) << err.what() << ": " << std::string(err.tokens());
        ASSERT_EQ(err.tokens().colnum(), 1) << err.what() << ": " << std::string(err.tokens());
    }
}

INSTANTIATE_TEST_CASE_P(semanticErrors, Resolver, ::testing::Values(
    "package test; auto foo(int x) {\nbool x; return x;}", // argument redifinition
    "package test; auto foo() {int x = 0; \nbool x; return x;}", // shadow another var
    "package test; auto foo(int x) {\nbool b; return x;}", // never used
    "package test; int foo(int x) {return x;} \nint foo(int x) {return x*x;}" // multiple function definitions
));
