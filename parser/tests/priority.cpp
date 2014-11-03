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

#include <cassert>
#include <vector>

#include <gtest/gtest.h>

#include "parser/actions.h"
#include "parser/binaryop.h"
#include "parser/metaparser.h"

namespace {

struct TestData
{
    const char *expr;
    std::vector<meta::BinaryOp::Operation> opSequence;
};

class Priority: public ::testing::TestWithParam<TestData>
{
public:
};

}

TEST_P(Priority, priority)
{
    const auto &param = GetParam();
    const char *tmpl = "package test; int foo() {return %s;}";
    char input[sizeof(tmpl) + strlen(param.expr)];
    sprintf(input, tmpl, param.expr);
    meta::Parser parser;
    meta::Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    std::vector<meta::BinaryOp::Operation> opSequence;
    meta::walkBottomUp<meta::BinaryOp>(*ast, [&](meta::BinaryOp *node) {opSequence.push_back(node->operation());});
    ASSERT_EQ(opSequence, param.opSequence);
}

INSTANTIATE_TEST_CASE_P(Operations, Priority, ::testing::Values(
    TestData({"1+2-3+4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::add, meta::BinaryOp::sub, meta::BinaryOp::add})}),
    TestData({"1-2+3-4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::sub, meta::BinaryOp::add, meta::BinaryOp::sub})}),

    TestData({"1+2*3+4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::add, meta::BinaryOp::add})}),
    TestData({"1*2+3*4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::mul, meta::BinaryOp::add})}),

    TestData({"1+2/3+4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::div, meta::BinaryOp::add, meta::BinaryOp::add})}),
    TestData({"1/2+3/4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::div, meta::BinaryOp::div, meta::BinaryOp::add})}),

    TestData({"1-2/3-4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::div, meta::BinaryOp::sub, meta::BinaryOp::sub})}),
    TestData({"1/2-3/4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::div, meta::BinaryOp::div, meta::BinaryOp::sub})}),

    TestData({"1-2*3-4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::sub, meta::BinaryOp::sub})}),
    TestData({"1*2-3*4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::mul, meta::BinaryOp::sub})}),

    TestData({"1*2/3*4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::div, meta::BinaryOp::mul})}),
    TestData({"1/2*3/4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::div, meta::BinaryOp::mul, meta::BinaryOp::div})}),

    TestData({"1+2*3>3-4/5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::add, meta::BinaryOp::div, meta::BinaryOp::sub, meta::BinaryOp::greater})}),
    TestData({"1+2*3>=3-4/5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::add, meta::BinaryOp::div, meta::BinaryOp::sub, meta::BinaryOp::greatereq})}),
    TestData({"1+2*3<3-4/5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::add, meta::BinaryOp::div, meta::BinaryOp::sub, meta::BinaryOp::less})}),
    TestData({"1+2*3<=3-4/5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::add, meta::BinaryOp::div, meta::BinaryOp::sub, meta::BinaryOp::lesseq})}),
    TestData({"1+2*3==3-4/5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::add, meta::BinaryOp::div, meta::BinaryOp::sub, meta::BinaryOp::equal})}),
    TestData({"1+2*3!=3-4/5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::mul, meta::BinaryOp::add, meta::BinaryOp::div, meta::BinaryOp::sub, meta::BinaryOp::noteq})}),

    TestData({"1<2 == 3<=5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::less, meta::BinaryOp::lesseq, meta::BinaryOp::equal})}),
    TestData({"1>2 == 3>=5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::greater, meta::BinaryOp::greatereq, meta::BinaryOp::equal})}),
    TestData({"1<2 != 3<=5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::less, meta::BinaryOp::lesseq, meta::BinaryOp::noteq})}),
    TestData({"1>2 != 3>=5", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::greater, meta::BinaryOp::greatereq, meta::BinaryOp::noteq})}),

    TestData({"1 == 2 != 3 == 4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::equal, meta::BinaryOp::noteq, meta::BinaryOp::equal})}),
    TestData({"1 != 2 == 3 != 4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::noteq, meta::BinaryOp::equal, meta::BinaryOp::noteq})}),

    TestData({"1 < 2 && 3 != 4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::less, meta::BinaryOp::noteq, meta::BinaryOp::boolAnd})}),
    TestData({"1+3 != 2 || 3 <= 4", std::vector<meta::BinaryOp::Operation>({meta::BinaryOp::add, meta::BinaryOp::noteq, meta::BinaryOp::lesseq, meta::BinaryOp::boolOr})})
));
