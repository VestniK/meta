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

#include "analysers/reachabilitychecker.h"
#include "analysers/semanticerror.h"

#include "parser/actions.h"
#include "parser/metaparser.h"

namespace {

class Reachability: public testing::TestWithParam<const char *>
{
public:
};

}

TEST_P(Reachability, resolveErrors) {
    const char *input = GetParam();
    meta::Parser parser;
    meta::Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    try {
        analysers::checkReachability(ast);
        ASSERT_TRUE(false) << "Input code contains unrachable statements however check didn't find them";
    } catch (analysers::SemanticError &err) {
        ASSERT_EQ(err.tokens().linenum(), 2) << err.what() << ": " << std::string(err.tokens());
        ASSERT_EQ(err.tokens().colnum(), 1) << err.what() << ": " << std::string(err.tokens());
    }
}

INSTANTIATE_TEST_CASE_P(semanticErrors, Reachability, ::testing::Values(
    "package test; auto foo(int x) {return x;\nbool y = x > 0;}",
    "package test; auto foo(int x) {int y = 2*x; {y = y + 5; return y;} \nreturn 0;}",
    "package test; auto foo(int x) {if (x < 0) return -x; else {int y = 5 + x; return y;\ny = y + x;} return 0;}"
));
