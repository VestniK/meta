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
#include "parser/number.h"
#include "parser/prefixop.h"

using namespace meta;

using namespace std::literals;

struct Operation
{
    BinaryOp::Operation operation;
    int left, right;
};

class LoggingCalc: public Visitor
{
public:
    virtual void leave(Number *num)
    {
        mCalcStack.push_back(num->value());
    }

    virtual void leave(BinaryOp *op)
    {
        assert(mCalcStack.size() >= 2);
        mCalcSequence.push_back(Operation());
        Operation &current = mCalcSequence.back();
        current.operation = op->operation();
        current.right = mCalcStack.back();
        mCalcStack.pop_back();
        current.left = mCalcStack.back();
        switch (current.operation) {
            case BinaryOp::add: mCalcStack.back() = current.left + current.right; break;
            case BinaryOp::sub: mCalcStack.back() = current.left - current.right; break;
            case BinaryOp::mul: mCalcStack.back() = current.left*current.right; break;
            case BinaryOp::div: mCalcStack.back() = current.left/current.right; break;
            default: assert(false);
        }
    }

    virtual void leave(PrefixOp *op)
    {
        assert(mCalcStack.size() >= 1);
        switch(op->operation()) {
            case PrefixOp::negative: mCalcStack.back() = - mCalcStack.back(); break;
            case PrefixOp::positive: mCalcStack.back() = + mCalcStack.back(); break;
            default: assert(false);
        }
    }

    const std::vector<Operation> &calcSequence() const {return mCalcSequence;}
    int result() const
    {
        assert(mCalcStack.size() == 1);
        return mCalcStack.back();
    }

private:
    std::vector<int> mCalcStack;
    std::vector<Operation> mCalcSequence;
};

TEST(Arythmetic, parenthesis)
{
    const auto input = "package test; int foo() {return 2*(11+5)/8;}"s;
    Parser parser;
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    LoggingCalc calc;
    ast->walk(&calc);
    ASSERT_EQ(calc.calcSequence().size(), 3u);

    ASSERT_EQ(calc.calcSequence()[0].operation, BinaryOp::add);
    ASSERT_EQ(calc.calcSequence()[0].left, 11);
    ASSERT_EQ(calc.calcSequence()[0].right, 5);

    ASSERT_EQ(calc.calcSequence()[1].operation, BinaryOp::mul);
    ASSERT_EQ(calc.calcSequence()[1].left, 2);
    ASSERT_EQ(calc.calcSequence()[1].right, 16);

    ASSERT_EQ(calc.calcSequence()[2].operation, BinaryOp::div);
    ASSERT_EQ(calc.calcSequence()[2].left, 32);
    ASSERT_EQ(calc.calcSequence()[2].right, 8);

    ASSERT_EQ(calc.result(), 4);
}

namespace {

struct TestData
{
    const char *expression;
    int expectedResult;
};

class Arythmetic: public ::testing::TestWithParam<TestData>
{
public:
};

}

TEST_P(Arythmetic, calcTest)
{
    TestData data = GetParam();
    const std::string input = str(boost::format("package test; int foo() {return %s;}")%data.expression);

    Parser parser;
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    LoggingCalc calc;
    ast->walk(&calc);
    ASSERT_EQ(calc.result(), data.expectedResult);
}

INSTANTIATE_TEST_CASE_P(PrefixOp, Arythmetic, ::testing::Values(
    TestData({"+1", 1}),
    TestData({"-1", -1}),
    TestData({"+1+5", 6}),
    TestData({"-1 + 5", 4}),
    TestData({"1 + +5", 6}),
    TestData({"1 + -5", -4}),
    TestData({"-2*3 + 4", -2}),
    TestData({"2*-3 + 4", -2}),
    TestData({"2*3 + -4", 2})
));
