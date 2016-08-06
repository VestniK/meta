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

#include <boost/format.hpp>

#include <gtest/gtest.h>

#include "parser/binaryop.h"
#include "parser/metaparser.h"

using namespace meta;

namespace {

struct TestData
{
    const char *expr;
    std::vector<BinaryOp::Operation> opSequence;
};

class Priority: public ::testing::TestWithParam<TestData>
{
public:
};

}

TEST_P(Priority, priority)
{
    const auto &param = GetParam();
    const std::string input = str(boost::format("package test; int foo() {return %s;}")%param.expr);
    Parser parser;
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    std::vector<BinaryOp::Operation> opSequence;
    walk<BinaryOp, BottomUp>(*ast, [&](BinaryOp *node) {opSequence.push_back(node->operation());});
    ASSERT_EQ(opSequence, param.opSequence);
}

INSTANTIATE_TEST_CASE_P(Operations, Priority, ::testing::Values(
    TestData({"1+2-3+4", std::vector<BinaryOp::Operation>({BinaryOp::add, BinaryOp::sub, BinaryOp::add})}),
    TestData({"1-2+3-4", std::vector<BinaryOp::Operation>({BinaryOp::sub, BinaryOp::add, BinaryOp::sub})}),

    TestData({"1+2*3+4", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::add, BinaryOp::add})}),
    TestData({"1*2+3*4", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::mul, BinaryOp::add})}),

    TestData({"1+2/3+4", std::vector<BinaryOp::Operation>({BinaryOp::div, BinaryOp::add, BinaryOp::add})}),
    TestData({"1/2+3/4", std::vector<BinaryOp::Operation>({BinaryOp::div, BinaryOp::div, BinaryOp::add})}),

    TestData({"1-2/3-4", std::vector<BinaryOp::Operation>({BinaryOp::div, BinaryOp::sub, BinaryOp::sub})}),
    TestData({"1/2-3/4", std::vector<BinaryOp::Operation>({BinaryOp::div, BinaryOp::div, BinaryOp::sub})}),

    TestData({"1-2*3-4", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::sub, BinaryOp::sub})}),
    TestData({"1*2-3*4", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::mul, BinaryOp::sub})}),

    TestData({"1*2/3*4", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::div, BinaryOp::mul})}),
    TestData({"1/2*3/4", std::vector<BinaryOp::Operation>({BinaryOp::div, BinaryOp::mul, BinaryOp::div})}),

    TestData({"1+2*3>3-4/5", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::greater})}),
    TestData({"1+2*3>=3-4/5", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::greatereq})}),
    TestData({"1+2*3<3-4/5", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::less})}),
    TestData({"1+2*3<=3-4/5", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::lesseq})}),
    TestData({"1+2*3==3-4/5", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::equal})}),
    TestData({"1+2*3!=3-4/5", std::vector<BinaryOp::Operation>({BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::noteq})}),

    TestData({"1<2 == 3<=5", std::vector<BinaryOp::Operation>({BinaryOp::less, BinaryOp::lesseq, BinaryOp::equal})}),
    TestData({"1>2 == 3>=5", std::vector<BinaryOp::Operation>({BinaryOp::greater, BinaryOp::greatereq, BinaryOp::equal})}),
    TestData({"1<2 != 3<=5", std::vector<BinaryOp::Operation>({BinaryOp::less, BinaryOp::lesseq, BinaryOp::noteq})}),
    TestData({"1>2 != 3>=5", std::vector<BinaryOp::Operation>({BinaryOp::greater, BinaryOp::greatereq, BinaryOp::noteq})}),

    TestData({"1 == 2 != 3 == 4", std::vector<BinaryOp::Operation>({BinaryOp::equal, BinaryOp::noteq, BinaryOp::equal})}),
    TestData({"1 != 2 == 3 != 4", std::vector<BinaryOp::Operation>({BinaryOp::noteq, BinaryOp::equal, BinaryOp::noteq})}),

    TestData({"1 < 2 && 3 != 4", std::vector<BinaryOp::Operation>({BinaryOp::less, BinaryOp::noteq, BinaryOp::boolAnd})}),
    TestData({"1+3 != 2 || 3 <= 4", std::vector<BinaryOp::Operation>({BinaryOp::add, BinaryOp::noteq, BinaryOp::lesseq, BinaryOp::boolOr})})
));
