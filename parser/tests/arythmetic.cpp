#include <cassert>
#include <vector>

#include <gtest/gtest.h>

#include "parser/binaryop.h"
#include "parser/number.h"

struct Operation
{
    meta::BinaryOp::Operation operation;
    int left, right;
};

class LoggingCalc: public meta::Visitor
{
public:
    virtual void leave(meta::Number *num)
    {
        mCalcStack.push_back(num->value());
    }

    virtual void leave(meta::BinaryOp *op)
    {
        assert(mCalcStack.size() >= 2);
        mCalcSequence.push_back(Operation());
        Operation &current = mCalcSequence.back();
        current.operation = op->operation();
        current.right = mCalcStack.back();
        mCalcStack.pop_back();
        current.left = mCalcStack.back();
        switch (current.operation) {
            case meta::BinaryOp::add: mCalcStack.back() = current.left + current.right; break;
            case meta::BinaryOp::sub: mCalcStack.back() = current.left - current.right; break;
            case meta::BinaryOp::mul: mCalcStack.back() = current.left*current.right; break;
            case meta::BinaryOp::div: mCalcStack.back() = current.left/current.right; break;
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

TEST(Arythmetic, priorities)
{
    const char *input = "package test; int foo() {return 5*2+7*8;}";
    meta::Parser parser;
    meta::AST ast;
    ASSERT_NO_THROW(ast = parser.parse(input));
    LoggingCalc calc;
    ast.walk(&calc);
    ASSERT_EQ(calc.calcSequence().size(), 3);

    ASSERT_EQ(calc.calcSequence()[0].operation, meta::BinaryOp::mul);
    ASSERT_EQ(calc.calcSequence()[0].left, 5);
    ASSERT_EQ(calc.calcSequence()[0].right, 2);

    ASSERT_EQ(calc.calcSequence()[1].operation, meta::BinaryOp::mul);
    ASSERT_EQ(calc.calcSequence()[1].left, 7);
    ASSERT_EQ(calc.calcSequence()[1].right, 8);

    ASSERT_EQ(calc.calcSequence()[2].operation, meta::BinaryOp::add);
    ASSERT_EQ(calc.calcSequence()[2].left, 10);
    ASSERT_EQ(calc.calcSequence()[2].right, 56);

    ASSERT_EQ(calc.result(), 66);
}

TEST(Arythmetic, parenthesis)
{
    const char *input = "package test; int foo() {return 2*(11+5)/8;}";
    meta::Parser parser;
    meta::AST ast;
    ASSERT_NO_THROW(ast = parser.parse(input));
    LoggingCalc calc;
    ast.walk(&calc);
    ASSERT_EQ(calc.calcSequence().size(), 3);

    ASSERT_EQ(calc.calcSequence()[0].operation, meta::BinaryOp::add);
    ASSERT_EQ(calc.calcSequence()[0].left, 11);
    ASSERT_EQ(calc.calcSequence()[0].right, 5);

    ASSERT_EQ(calc.calcSequence()[1].operation, meta::BinaryOp::mul);
    ASSERT_EQ(calc.calcSequence()[1].left, 2);
    ASSERT_EQ(calc.calcSequence()[1].right, 16);

    ASSERT_EQ(calc.calcSequence()[2].operation, meta::BinaryOp::div);
    ASSERT_EQ(calc.calcSequence()[2].left, 32);
    ASSERT_EQ(calc.calcSequence()[2].right, 8);

    ASSERT_EQ(calc.result(), 4);
}
