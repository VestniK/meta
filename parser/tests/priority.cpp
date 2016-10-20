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

#include "utils/testtools.h"

#include "parser/binaryop.h"
#include "parser/metaparser.h"

namespace meta::parser {
namespace {

struct TestData
{
    const char* expr;
    std::vector<BinaryOp::Operation> opSequence;
};

class Priority: public ::testing::TestWithParam<TestData>
{};

TEST_P(Priority, priority)
{
    const auto &param = GetParam();
    const auto input = utils::SourceFile::fake(
        str(boost::format("package test; int foo() {return %s;}")%param.expr)
    );
    Parser parser;
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();
    std::vector<BinaryOp::Operation> opSequence;
    walk<BinaryOp, BottomUp>(*ast, [&](BinaryOp *node) {opSequence.push_back(node->operation());});
    ASSERT_EQ(opSequence, param.opSequence);
}

TestData dataset[] = {
    {"1+2-3+4", {BinaryOp::add, BinaryOp::sub, BinaryOp::add}},
    {"1-2+3-4", {BinaryOp::sub, BinaryOp::add, BinaryOp::sub}},

    {"1+2*3+4", {BinaryOp::mul, BinaryOp::add, BinaryOp::add}},
    {"1*2+3*4", {BinaryOp::mul, BinaryOp::mul, BinaryOp::add}},

    {"1+2/3+4", {BinaryOp::div, BinaryOp::add, BinaryOp::add}},
    {"1/2+3/4", {BinaryOp::div, BinaryOp::div, BinaryOp::add}},

    {"1-2/3-4", {BinaryOp::div, BinaryOp::sub, BinaryOp::sub}},
    {"1/2-3/4", {BinaryOp::div, BinaryOp::div, BinaryOp::sub}},

    {"1-2*3-4", {BinaryOp::mul, BinaryOp::sub, BinaryOp::sub}},
    {"1*2-3*4", {BinaryOp::mul, BinaryOp::mul, BinaryOp::sub}},

    {"1*2/3*4", {BinaryOp::mul, BinaryOp::div, BinaryOp::mul}},
    {"1/2*3/4", {BinaryOp::div, BinaryOp::mul, BinaryOp::div}},

    {"1+2*3>3-4/5", {BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::greater}},
    {"1+2*3>=3-4/5", {BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::greatereq}},
    {"1+2*3<3-4/5", {BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::less}},
    {"1+2*3<=3-4/5", {BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::lesseq}},
    {"1+2*3==3-4/5", {BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::equal}},
    {"1+2*3!=3-4/5", {BinaryOp::mul, BinaryOp::add, BinaryOp::div, BinaryOp::sub, BinaryOp::noteq}},

    {"1<2 == 3<=5", {BinaryOp::less, BinaryOp::lesseq, BinaryOp::equal}},
    {"1>2 == 3>=5", {BinaryOp::greater, BinaryOp::greatereq, BinaryOp::equal}},
    {"1<2 != 3<=5", {BinaryOp::less, BinaryOp::lesseq, BinaryOp::noteq}},
    {"1>2 != 3>=5", {BinaryOp::greater, BinaryOp::greatereq, BinaryOp::noteq}},

    {"1 == 2 != 3 == 4", {BinaryOp::equal, BinaryOp::noteq, BinaryOp::equal}},
    {"1 != 2 == 3 != 4", {BinaryOp::noteq, BinaryOp::equal, BinaryOp::noteq}},

    {"1 < 2 && 3 != 4", {BinaryOp::less, BinaryOp::noteq, BinaryOp::boolAnd}},
    {"1+3 != 2 || 3 <= 4", {BinaryOp::add, BinaryOp::noteq, BinaryOp::lesseq, BinaryOp::boolOr}}
};
INSTANTIATE_TEST_CASE_P(Operations, Priority, ::testing::ValuesIn(dataset));

} // anonymous namespace
} // namesoace meta::parser
