/*
 * Meta language compiler
 * Copyright (C) 2016  Sergey Vidyuk <sir.vestnik@gmail.com>
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
#include <string>

#include <gtest/gtest.h>

#include "parser/actions.h"
#include "parser/annotation.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/vardecl.h"

using namespace meta;

using namespace std::literals;

/**
 * @class meta::Parser
 * @test This test checks that all possible function delaration syntaxes are parsed correctly and visibility
 * is set properly for functions with and with no annotations.
 */
TEST(FunctionParsing, funcDeclarationsAndVisibilities)
{
    const auto input = R"META(
        package test;

        int privateByDefault() {return 5;}

        protected:

        int protectedByModifiedDefault1() {return 5;}

        public int pubExplicitly() {return 5;}

        @some
        int protectedByModifiedDefault2() {return 5;}

        @some
        @other
        private int privateExplicitly() {return 5;}
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 5u);

    ASSERT_EQ(functions[0]->name(), "privateByDefault");
    ASSERT_EQ(functions[0]->visibility(), Visibility::Private);
    ASSERT_EQ(functions[0]->getChildren<Annotation>().size(), 0u);

    ASSERT_EQ(functions[1]->name(), "protectedByModifiedDefault1");
    ASSERT_EQ(functions[1]->visibility(), Visibility::Protected);
    ASSERT_EQ(functions[1]->getChildren<Annotation>().size(), 0u);

    ASSERT_EQ(functions[2]->name(), "pubExplicitly");
    ASSERT_EQ(functions[2]->visibility(), Visibility::Public);
    ASSERT_EQ(functions[2]->getChildren<Annotation>().size(), 0u);

    ASSERT_EQ(functions[3]->name(), "protectedByModifiedDefault2");
    ASSERT_EQ(functions[3]->visibility(), Visibility::Protected);
    ASSERT_EQ(functions[3]->getChildren<Annotation>().size(), 1u);
    ASSERT_EQ(functions[3]->getChildren<Annotation>()[0]->name(), "some");
    ASSERT_EQ(functions[3]->getChildren<Annotation>()[0]->target(), functions[3]);

    ASSERT_EQ(functions[4]->name(), "privateExplicitly");
    ASSERT_EQ(functions[4]->visibility(), Visibility::Private);
    ASSERT_EQ(functions[4]->getChildren<Annotation>().size(), 2u);
    ASSERT_EQ(functions[4]->getChildren<Annotation>()[0]->name(), "some");
    ASSERT_EQ(functions[4]->getChildren<Annotation>()[0]->target(), functions[4]);
    ASSERT_EQ(functions[4]->getChildren<Annotation>()[1]->name(), "other");
    ASSERT_EQ(functions[4]->getChildren<Annotation>()[1]->target(), functions[4]);
}

TEST(FunctionParsing, zeroParamFunc) {
    const auto input = "package test; int foo() {return 5;}"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 1u);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    ASSERT_EQ(functions[0]->args().size(), 0u);
}

TEST(FunctionParsing, oneParamFunc) {
    const auto input = "package test; int foo(int x) {return 5*x*x - 2*x + 3;}"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 1u);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 1u);
    ASSERT_EQ(args[0]->typeName(), "int");
    ASSERT_EQ(args[0]->name(), "x");
}

TEST(FunctionParsing, twoParamFunc) {
    const auto input = "package test; int foo(int x, int y) {return 5*x + 6/y;}"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 1u);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 2u);
    ASSERT_EQ(args[0]->typeName(), "int");
    ASSERT_EQ(args[0]->name(), "x");
    ASSERT_EQ(args[1]->typeName(), "int");
    ASSERT_EQ(args[1]->name(), "y");
}

TEST(FunctionParsing, twoFunc) {
    const auto input = "package test; int foo(int x) {return 5*x;}\nint bar(int x) {return x/5;}"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 2u);

    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args1 = functions[0]->args();
    ASSERT_EQ(args1.size(), 1u);
    ASSERT_EQ(args1[0]->typeName(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(functions[1]->name(), "bar");
    ASSERT_EQ(functions[1]->package(), "test");
    const auto args2 = functions[1]->args();
    ASSERT_EQ(args2.size(), 1u);
    ASSERT_EQ(args2[0]->typeName(), "int");
    ASSERT_EQ(args2[0]->name(), "x");
}

TEST(FunctionParsing, funcCall) {
    const auto input = "package test; int foo(int x) {return 5*x;}\nint bar(int y) {return 5*foo(y/5);}"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 2u);

    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args1 = functions[0]->args();
    ASSERT_EQ(args1.size(), 1u);
    ASSERT_EQ(args1[0]->typeName(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(functions[1]->name(), "bar");
    ASSERT_EQ(functions[1]->package(), "test");
    const auto args2 = functions[1]->args();
    ASSERT_EQ(args2.size(), 1u);
    ASSERT_EQ(args2[0]->typeName(), "int");
    ASSERT_EQ(args2[0]->name(), "y");
}

TEST(FunctionParsing, funcRetType) {
    const auto input = "package example.test; int iFoo() {return 0;}\ndouble dFoo() {return 0;}"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 2u);
    ASSERT_EQ(functions[0]->name(), "iFoo");
    ASSERT_EQ(functions[0]->package(), "example.test");
    ASSERT_EQ(functions[0]->retType(), "int");
    ASSERT_EQ(functions[1]->name(), "dFoo");
    ASSERT_EQ(functions[1]->package(), "example.test");
    ASSERT_EQ(functions[1]->retType(), "double");
}
