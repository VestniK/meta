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
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "parser/actions.h"
#include "parser/assigment.h"
#include "parser/codeblock.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/vardecl.h"

TEST(Parser, zeroParamFunc) {
    const char *input = "package test; int foo() {return 5;}";
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
    const auto functions = ast->getChildren<meta::Function>(0);
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    ASSERT_EQ(functions[0]->args().size(), 0);
}

TEST(Parser, oneParamFunc) {
    const char *input = "package test; int foo(int x) {return 5*x*x - 2*x + 3;}";
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 1);
    ASSERT_EQ(args[0]->type(), "int");
    ASSERT_EQ(args[0]->name(), "x");
}

TEST(Parser, twoParamFunc) {
    const char *input = "package test; int foo(int x, int y) {return 5*x + 6/y;}";
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 2);
    ASSERT_EQ(args[0]->type(), "int");
    ASSERT_EQ(args[0]->name(), "x");
    ASSERT_EQ(args[1]->type(), "int");
    ASSERT_EQ(args[1]->name(), "y");
}

TEST(Parser, twoFunc) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int x) {return x/5;}";
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 2);

    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args1 = functions[0]->args();
    ASSERT_EQ(args1.size(), 1);
    ASSERT_EQ(args1[0]->type(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(functions[1]->name(), "bar");
    ASSERT_EQ(functions[1]->package(), "test");
    const auto args2 = functions[1]->args();
    ASSERT_EQ(args2.size(), 1);
    ASSERT_EQ(args2[0]->type(), "int");
    ASSERT_EQ(args2[0]->name(), "x");
}

TEST(Parser, funcCall) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int y) {return 5*foo(y/5);}";
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 2);

    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args1 = functions[0]->args();
    ASSERT_EQ(args1.size(), 1);
    ASSERT_EQ(args1[0]->type(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(functions[1]->name(), "bar");
    ASSERT_EQ(functions[1]->package(), "test");
    const auto args2 = functions[1]->args();
    ASSERT_EQ(args2.size(), 1);
    ASSERT_EQ(args2[0]->type(), "int");
    ASSERT_EQ(args2[0]->name(), "y");
}

TEST(Parser, emptyPackage) {
    const char *input = "package test.test;";
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
    const auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 0);
}

TEST(Parser, funcRetType) {
    const char *input = "package example.test; int iFoo() {return 0;}\ndouble dFoo() {return 0;}";
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
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
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
    auto blocks = ast->getChildren<meta::CodeBlock>(-1);
    ASSERT_EQ(blocks.size(), 1);
    auto varDeclarations = blocks.front()->getChildren<meta::VarDecl>(-1);
    ASSERT_EQ(varDeclarations.size(), 2);
    ASSERT_EQ(varDeclarations[0]->type(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_TRUE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->type(), "int");
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
    meta::Parser parser;
    Actions act;
    parser.setParseActions(&act);
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parser.parse(input)));
    auto blocks = ast->getChildren<meta::CodeBlock>(-1);
    ASSERT_EQ(blocks.size(), 1);
    auto varDeclarations = blocks.front()->getChildren<meta::VarDecl>(-1);
    ASSERT_EQ(varDeclarations.size(), 2);
    ASSERT_EQ(varDeclarations[0]->type(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_FALSE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->type(), "int");
    ASSERT_EQ(varDeclarations[1]->name(), "z");
    ASSERT_FALSE(varDeclarations[1]->inited());

    auto assigments = blocks.front()->getChildren<meta::Assigment>(-1);
    ASSERT_EQ(assigments.size(), 2);
    ASSERT_EQ(assigments[0]->varName(), "z");
    ASSERT_EQ(assigments[1]->varName(), "y");
}
