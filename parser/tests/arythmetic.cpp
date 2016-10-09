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

#include "parser/metanodes.h"
#include "parser/unexpectednode.h"

namespace meta::parser::tests {
namespace {

struct Operation {
    BinaryOp::Operation operation;
    int left, right;
};

class LoggingCalc {
public:
    const auto& calcSequence() const {return mCalcSequence;}

    int operator() (Node* node) {throw UnexpectedNode(node, "arythmetic expression required");}

    int operator() (Number* num) {return num->value();}

    int operator() (BinaryOp* op) {
        const int lhs = dispatch(*this, op->left());
        const int rhs = dispatch(*this, op->right());
        mCalcSequence.push_back({op->operation(), lhs, rhs});
        switch (op->operation()) {
            case BinaryOp::add: return lhs + rhs;
            case BinaryOp::sub: return lhs - rhs;
            case BinaryOp::mul: return lhs*rhs;
            case BinaryOp::div: return lhs/rhs;
            default: break;
        }
        throw UnexpectedNode(op, "arythmetic operation expected");
    }

    int operator() (PrefixOp* op) {
        const int operand = dispatch(*this, op->operand());
        switch (op->operation()) {
            case PrefixOp::negative: return -operand;
            case PrefixOp::positive: return +operand;
            default: break;
        }
        throw std::runtime_error("unexpected operation");
    }

private:
    std::vector<Operation> mCalcSequence;
};

template<typename Walkable>
auto findExpressions(Walkable* walkable) {
    std::vector<Expression*> res;
    walk<Node, TopDown>(walkable, [&res](Node* node) {
        auto* expr = dynamic_cast<Expression*>(node);
        if (!expr)
            return true; // continue tree traversal
        res.push_back(expr);
        return false; // Do not traverse children
    });
    return res;
}

TEST(Arythmetic, parenthesis) {
    const utils::SourceFile input = "package test; int foo() {return 2*(11+5)/8;}";
    Parser parser;
    ASSERT_PARSE(parser, input);
    const auto expressions = findExpressions(parser.ast());
    ASSERT_EQ(expressions.size(), 1u);
    LoggingCalc calc;
    const int calcRes = dispatch(calc, expressions[0]);
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

    ASSERT_EQ(calcRes, 4);
}

namespace {

struct TestData {
    const char *expression;
    int expectedResult;
};

class Arythmetic: public ::testing::TestWithParam<TestData> {};

}

TEST_P(Arythmetic, calcTest) {
    TestData data = GetParam();
    const auto input = utils::SourceFile::fake(
        "test.meta",
        str(boost::format("package test; int foo() {return %s;}")%data.expression)
    );

    Parser parser;
    ASSERT_PARSE(parser, input);
    const auto expressions = findExpressions(parser.ast());
    ASSERT_EQ(expressions.size(), 1u);
    LoggingCalc calc;
    const int calcRes = dispatch(calc, expressions[0]);
    ASSERT_EQ(calcRes, data.expectedResult);
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

} // anonymous namespace
} // namespace meta::parser::tests
