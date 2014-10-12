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
#include <cstdio>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "parser/assigment.h"
#include "parser/binaryop.h"
#include "parser/codeblock.h"
#include "parser/if.h"
#include "parser/function.h"
#include "parser/parse.h"
#include "parser/return.h"
#include "parser/vardecl.h"

TEST(Parser, zeroParamFunc) {
    const char *input = "package test; int foo() {return 5;}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    const auto functions = ast->getChildren<meta::Function>(0);
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    ASSERT_EQ(functions[0]->args().size(), 0);
}

TEST(Parser, oneParamFunc) {
    const char *input = "package test; int foo(int x) {return 5*x*x - 2*x + 3;}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 1);
    ASSERT_EQ(args[0]->typeName(), "int");
    ASSERT_EQ(args[0]->name(), "x");
}

TEST(Parser, twoParamFunc) {
    const char *input = "package test; int foo(int x, int y) {return 5*x + 6/y;}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 2);
    ASSERT_EQ(args[0]->typeName(), "int");
    ASSERT_EQ(args[0]->name(), "x");
    ASSERT_EQ(args[1]->typeName(), "int");
    ASSERT_EQ(args[1]->name(), "y");
}

TEST(Parser, twoFunc) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int x) {return x/5;}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 2);

    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args1 = functions[0]->args();
    ASSERT_EQ(args1.size(), 1);
    ASSERT_EQ(args1[0]->typeName(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(functions[1]->name(), "bar");
    ASSERT_EQ(functions[1]->package(), "test");
    const auto args2 = functions[1]->args();
    ASSERT_EQ(args2.size(), 1);
    ASSERT_EQ(args2[0]->typeName(), "int");
    ASSERT_EQ(args2[0]->name(), "x");
}

TEST(Parser, funcCall) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int y) {return 5*foo(y/5);}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 2);

    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args1 = functions[0]->args();
    ASSERT_EQ(args1.size(), 1);
    ASSERT_EQ(args1[0]->typeName(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(functions[1]->name(), "bar");
    ASSERT_EQ(functions[1]->package(), "test");
    const auto args2 = functions[1]->args();
    ASSERT_EQ(args2.size(), 1);
    ASSERT_EQ(args2[0]->typeName(), "int");
    ASSERT_EQ(args2[0]->name(), "y");
}

TEST(Parser, emptyPackage) {
    const char *input = "package test.test;";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 0);
}

TEST(Parser, funcRetType) {
    const char *input = "package example.test; int iFoo() {return 0;}\ndouble dFoo() {return 0;}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 2);
    ASSERT_EQ(functions[0]->name(), "iFoo");
    ASSERT_EQ(functions[0]->package(), "example.test");
    ASSERT_EQ(functions[0]->retType(), "int");
    ASSERT_EQ(functions[1]->name(), "dFoo");
    ASSERT_EQ(functions[1]->package(), "example.test");
    ASSERT_EQ(functions[1]->retType(), "double");
}

TEST(Parser, varTest) {
    const char *input = R"META(
        package test;
        int foo(int x)
        {
            int y = 2*x;
            int z;
            z = x*x;
            return z + y - 3;
        }
    )META";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    auto blocks = ast->getChildren<meta::CodeBlock>(-1);
    ASSERT_EQ(blocks.size(), 1);
    auto varDeclarations = blocks.front()->getChildren<meta::VarDecl>(-1);
    ASSERT_EQ(varDeclarations.size(), 2);
    ASSERT_EQ(varDeclarations[0]->typeName(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_TRUE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->typeName(), "int");
    ASSERT_EQ(varDeclarations[1]->name(), "z");
    ASSERT_FALSE(varDeclarations[1]->inited());

    auto assigments = blocks.front()->getChildren<meta::Assigment>(-1);
    ASSERT_EQ(assigments.size(), 1);
    ASSERT_EQ(assigments[0]->varName(), "z");
}

TEST(Parser, assignAsExpr) {
    const char *input = R"META(
        package test;
        int foo(int x)
        {
            int y;
            int z;
            z = (y = x + 1)*x;
            return z + y - 3;
        }
    )META";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    auto blocks = ast->getChildren<meta::CodeBlock>(-1);
    ASSERT_EQ(blocks.size(), 1);
    auto varDeclarations = blocks.front()->getChildren<meta::VarDecl>(-1);
    ASSERT_EQ(varDeclarations.size(), 2);
    ASSERT_EQ(varDeclarations[0]->typeName(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_FALSE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->typeName(), "int");
    ASSERT_EQ(varDeclarations[1]->name(), "z");
    ASSERT_FALSE(varDeclarations[1]->inited());

    auto assigments = blocks.front()->getChildren<meta::Assigment>(-1);
    ASSERT_EQ(assigments.size(), 2);
    ASSERT_EQ(assigments[0]->varName(), "z");
    ASSERT_EQ(assigments[1]->varName(), "y");
}

TEST(Parser, ifStatement) {
    const char *input = R"META(
        package test;
        int abs(int x)
        {
            if (x < 0)
                return -x;
            return x;
        }
    )META";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    auto ifs = ast->getChildren<meta::If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto returns = ast->getChildren<meta::Return>(-1);
    auto condRets = ifs[0]->getChildren<meta::Return>(-1);
    ASSERT_EQ(returns.size(), 2);
    ASSERT_EQ(condRets.size(), 1);
    ASSERT_EQ(condRets[0], returns[0]);
    ASSERT_NE(condRets[0], returns[1]);
}

TEST(Parser, ifElseStatement) {
    const char *input = R"META(
        package test;
        int abs(int x)
        {
            if (x < 0)
                return -x;
            else
                return x;
            return 0;
        }
    )META";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    auto ifs = ast->getChildren<meta::If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto returns = ast->getChildren<meta::Return>(-1);
    auto condRets = ifs[0]->getChildren<meta::Return>(-1);
    ASSERT_EQ(returns.size(), 3);
    ASSERT_EQ(condRets.size(), 2);

    ASSERT_EQ(condRets[0], returns[0]);
    ASSERT_NE(condRets[0], returns[2]);

    ASSERT_EQ(condRets[1], returns[1]);
    ASSERT_NE(condRets[1], returns[2]);
}

TEST(Parser, ifBlockStatement) {
    const char *input = R"META(
        package test;
        int foo(int x)
        {
            int y = x;
            if (x < 0) {
                y = -x;
                y = y + 1;
            }
            y = y + 10;
            return y;
        }
    )META";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    auto ifs = ast->getChildren<meta::If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto assigns = ast->getChildren<meta::Assigment>(-1);
    auto condAssigns = ifs[0]->getChildren<meta::Assigment>(-1);
    ASSERT_EQ(assigns.size(), 3);
    ASSERT_EQ(condAssigns.size(), 2);

    ASSERT_EQ(condAssigns[0], assigns[0]);
    ASSERT_NE(condAssigns[0], assigns[2]);

    ASSERT_EQ(condAssigns[1], assigns[1]);
    ASSERT_NE(condAssigns[1], assigns[2]);
}

TEST(Parser, ifElseBlockStatement) {
    const char *input = R"META(
        package test;
        int foo(int x)
        {
            int y = x;
            if (x < 0) {
                y = -x;
                y = y + 1;
            } else {
                y = y + 2;
                y = y/2;
            }
            y = y + 10;
            return y;
        }
    )META";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    auto ifs = ast->getChildren<meta::If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto assigns = ast->getChildren<meta::Assigment>(-1);
    auto condAssigns = ifs[0]->getChildren<meta::Assigment>(-1);
    ASSERT_EQ(assigns.size(), 5);
    ASSERT_EQ(condAssigns.size(), 4);

    for (size_t pos = 0; pos < condAssigns.size(); ++pos) {
        ASSERT_EQ(condAssigns[pos], assigns[pos]);
        ASSERT_NE(condAssigns[pos], assigns[assigns.size() - 1]);
    }
}

namespace {

struct TestData
{
    const char *opStr;
    meta::BinaryOp::Operation op;
};

class Parser: public testing::TestWithParam<TestData>
{
};

}

TEST_P(Parser, binaryOp) {
    TestData data = GetParam();
    const char *tmplt = "package test; int foo(int x, int y) {return x %s y;}";
    char input[sizeof(tmplt) + strlen(data.opStr)];
    sprintf(input, tmplt, data.opStr);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    auto binops = ast->getChildren<meta::BinaryOp>(-1);
    ASSERT_EQ(binops.size(), 1);
    ASSERT_EQ(binops[0]->operation(), data.op);
}

INSTANTIATE_TEST_CASE_P(BiaryOps, Parser, ::testing::Values(
    TestData({"+", meta::BinaryOp::add}),
    TestData({"-", meta::BinaryOp::sub}),
    TestData({"*", meta::BinaryOp::mul}),
    TestData({"/", meta::BinaryOp::div}),
    TestData({"==", meta::BinaryOp::equal}),
    TestData({"!=", meta::BinaryOp::noteq}),
    TestData({"<", meta::BinaryOp::less}),
    TestData({">", meta::BinaryOp::greater}),
    TestData({"<=", meta::BinaryOp::lesseq}),
    TestData({">=", meta::BinaryOp::greatereq})
));
