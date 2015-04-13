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

#include "parser/actions.h"
#include "parser/annotation.h"
#include "parser/assigment.h"
#include "parser/binaryop.h"
#include "parser/call.h"
#include "parser/codeblock.h"
#include "parser/if.h"
#include "parser/import.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/strliteral.h"
#include "parser/return.h"
#include "parser/vardecl.h"

using namespace meta;

/**
 * @class meta::Parser
 * @test This test checks that all possible function delaration syntaxes are parsed correctly and visibility
 * is set properly for functions with and with no annotations.
 */
TEST(MetaParser, funcDeclarationsAndVisibilities)
{
    const char *input = R"META(
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
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 5);

    ASSERT_EQ(functions[0]->name(), "privateByDefault");
    ASSERT_EQ(functions[0]->visibility(), Visibility::Private);
    ASSERT_EQ(functions[0]->getChildren<Annotation>().size(), 0);

    ASSERT_EQ(functions[1]->name(), "protectedByModifiedDefault1");
    ASSERT_EQ(functions[1]->visibility(), Visibility::Protected);
    ASSERT_EQ(functions[1]->getChildren<Annotation>().size(), 0);

    ASSERT_EQ(functions[2]->name(), "pubExplicitly");
    ASSERT_EQ(functions[2]->visibility(), Visibility::Public);
    ASSERT_EQ(functions[2]->getChildren<Annotation>().size(), 0);

    ASSERT_EQ(functions[3]->name(), "protectedByModifiedDefault2");
    ASSERT_EQ(functions[3]->visibility(), Visibility::Protected);
    ASSERT_EQ(functions[3]->getChildren<Annotation>().size(), 1);
    ASSERT_EQ(functions[3]->getChildren<Annotation>()[0]->name(), "some");
    ASSERT_EQ(functions[3]->getChildren<Annotation>()[0]->target(), functions[3]);

    ASSERT_EQ(functions[4]->name(), "privateExplicitly");
    ASSERT_EQ(functions[4]->visibility(), Visibility::Private);
    ASSERT_EQ(functions[4]->getChildren<Annotation>().size(), 2);
    ASSERT_EQ(functions[4]->getChildren<Annotation>()[0]->name(), "some");
    ASSERT_EQ(functions[4]->getChildren<Annotation>()[0]->target(), functions[4]);
    ASSERT_EQ(functions[4]->getChildren<Annotation>()[1]->name(), "other");
    ASSERT_EQ(functions[4]->getChildren<Annotation>()[1]->target(), functions[4]);
}

TEST(MetaParser, zeroParamFunc) {
    const char *input = "package test; int foo() {return 5;}";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    ASSERT_EQ(functions[0]->args().size(), 0);
}

TEST(MetaParser, imports) {
    const char *input = "package test; import pkg.bar; import pkg.subpkg.bar as bar1; int foo() {return 5;}";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto imports = ast->getChildren<Import>(-1);
    ASSERT_EQ(imports.size(), 2);
    ASSERT_EQ(imports[0]->name(), "bar");
    ASSERT_EQ(imports[0]->target(), "bar");
    ASSERT_EQ(imports[0]->targetPackage(), "pkg");
    ASSERT_EQ(imports[1]->name(), "bar1");
    ASSERT_EQ(imports[1]->target(), "bar");
    ASSERT_EQ(imports[1]->targetPackage(), "pkg.subpkg");
}

TEST(MetaParser, oneParamFunc) {
    const char *input = "package test; int foo(int x) {return 5*x*x - 2*x + 3;}";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->package(), "test");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 1);
    ASSERT_EQ(args[0]->typeName(), "int");
    ASSERT_EQ(args[0]->name(), "x");
}

TEST(MetaParser, twoParamFunc) {
    const char *input = "package test; int foo(int x, int y) {return 5*x + 6/y;}";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
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

TEST(MetaParser, twoFunc) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int x) {return x/5;}";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
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

TEST(MetaParser, funcCall) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int y) {return 5*foo(y/5);}";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
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

TEST(MetaParser, emptyPackage) {
    const char *input = "package test.test;";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 0);
}

TEST(MetaParser, funcRetType) {
    const char *input = "package example.test; int iFoo() {return 0;}\ndouble dFoo() {return 0;}";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 2);
    ASSERT_EQ(functions[0]->name(), "iFoo");
    ASSERT_EQ(functions[0]->package(), "example.test");
    ASSERT_EQ(functions[0]->retType(), "int");
    ASSERT_EQ(functions[1]->name(), "dFoo");
    ASSERT_EQ(functions[1]->package(), "example.test");
    ASSERT_EQ(functions[1]->retType(), "double");
}

TEST(MetaParser, varTest) {
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
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto blocks = ast->getChildren<CodeBlock>(-1);
    ASSERT_EQ(blocks.size(), 1);
    auto varDeclarations = blocks.front()->getChildren<VarDecl>(-1);
    ASSERT_EQ(varDeclarations.size(), 2);
    ASSERT_EQ(varDeclarations[0]->typeName(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_TRUE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->typeName(), "int");
    ASSERT_EQ(varDeclarations[1]->name(), "z");
    ASSERT_FALSE(varDeclarations[1]->inited());

    auto assigments = blocks.front()->getChildren<Assigment>(-1);
    ASSERT_EQ(assigments.size(), 1);
    ASSERT_EQ(assigments[0]->varName(), "z");
}

TEST(MetaParser, assignAsExpr) {
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
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto blocks = ast->getChildren<CodeBlock>(-1);
    ASSERT_EQ(blocks.size(), 1);
    auto varDeclarations = blocks.front()->getChildren<VarDecl>(-1);
    ASSERT_EQ(varDeclarations.size(), 2);
    ASSERT_EQ(varDeclarations[0]->typeName(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_FALSE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->typeName(), "int");
    ASSERT_EQ(varDeclarations[1]->name(), "z");
    ASSERT_FALSE(varDeclarations[1]->inited());

    auto assigments = blocks.front()->getChildren<Assigment>(-1);
    ASSERT_EQ(assigments.size(), 2);
    ASSERT_EQ(assigments[0]->varName(), "z");
    ASSERT_EQ(assigments[1]->varName(), "y");
}

TEST(MetaParser, ifStatement) {
    const char *input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                foo1(x);
            foo2(x);
        }
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 2);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
}

TEST(MetaParser, ifWithEmptyStatement) {
    const char *input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                ;
            foo1(x);
        }
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 1);
    ASSERT_EQ(calls[0]->functionName(), "foo1");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
}

TEST(MetaParser, ifElseStatement) {
    const char *input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                foo1(x);
            else
                foo2(x);
            foo3(x);
        }
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 3);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");
    ASSERT_EQ(calls[2]->functionName(), "foo3");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_NE(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
    ASSERT_EQ(ifs[0]->elseBlock()->getChildren<Call>(-1).size(), 1);
    ASSERT_EQ(ifs[0]->elseBlock()->getChildren<Call>(-1)[0], calls[1]);
}

TEST(MetaParser, ifElseEmptyStatement) {
    const char *input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                foo1(x);
            else
                ;
            foo2(x);
        }
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 2);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
}

TEST(MetaParser, ifElseBothEmptyStatements) {
    const char *input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                ;
            else
                ;
            foo1(x);
        }
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 1);
    ASSERT_EQ(calls[0]->functionName(), "foo1");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
}

TEST(MetaParser, ifBlockStatement) {
    const char *input = R"META(
        package test;
        int foo0(int x)
        {
            int y = x;
            if (x < 0) {
                y = -x;
                y = foo1(y);
            }
            y = foo2(y);
            return y;
        }
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 2);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
}

TEST(MetaParser, ifElseBlockStatement) {
    const char *input = R"META(
        package test;
        int foo0(int x)
        {
            int y = x;
            if (x < 0) {
                y = -x;
                y = foo1(y);
            } else {
                y = y + 2;
                y = foo2(y);
            }
            y = foo3(y);
            return y;
        }
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 3);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");
    ASSERT_EQ(calls[2]->functionName(), "foo3");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_NE(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
    ASSERT_EQ(ifs[0]->elseBlock()->getChildren<Call>(-1).size(), 1);
    ASSERT_EQ(ifs[0]->elseBlock()->getChildren<Call>(-1)[0], calls[1]);
}

TEST(MetaParser, multipleFiles) {
    const char *src1 = "package test; int foo() {return 0;}";
    const char *src2 = "package test; bool bar() {return false;}";

    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    parser.setSourcePath("src1");
    ASSERT_NO_THROW(parser.parse(src1, strlen(src1)));
    parser.setSourcePath("src2");
    ASSERT_NO_THROW(parser.parse(src2, strlen(src2)));

    auto ast = parser.ast();
    Function *foo = nullptr;
    Function *bar = nullptr;
    walk<Function, TopDown>(*ast, [&foo, &bar](Function *node) {
        if (node->name() == "foo")
            foo = node;
        else if (node->name() == "bar")
            bar = node;
        return false;
    });
    ASSERT_NE(foo, nullptr);
    ASSERT_EQ(foo->sourcePath(), "src1");
    ASSERT_NE(bar, nullptr);
    ASSERT_EQ(bar->sourcePath(), "src2");
}

namespace {

template<size_t N>
inline std::vector<char> str2buf(const char (&str)[N])
{
    return std::vector<char>(str, str + N - 1); // ignore auto assigned zero terminator
}

}

TEST(Parser, stringLiteral) {
    const char *input = R"META(
        package test;
        string foo() {
            return "Hello World";
        }
        string escaped() {
            return "\t \n \r \a \b \f \\ \" \0";
        }
    )META";

    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));

    auto strs = parser.ast()->getChildren<StrLiteral>(-1);
    ASSERT_EQ(strs.size(), 2);
    ASSERT_EQ(strs[0]->value(), str2buf("Hello World"));
    ASSERT_EQ(strs[1]->value(), str2buf("\t \n \r \a \b \f \\ \" \0"));
}

namespace {

struct TestData
{
    const char *opStr;
    BinaryOp::Operation op;
};

class MetaParser: public testing::TestWithParam<TestData>
{
};

}

TEST_P(MetaParser, binaryOp) {
    TestData data = GetParam();
    const char *tmplt = "package test; int foo(int x, int y) {return x %s y;}";
    char input[strlen(tmplt) + strlen(data.opStr)];
    sprintf(input, tmplt, data.opStr);
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse(input, strlen(input)));
    auto ast = parser.ast();
    auto binops = ast->getChildren<BinaryOp>(-1);
    ASSERT_EQ(binops.size(), 1);
    ASSERT_EQ(binops[0]->operation(), data.op);
}

INSTANTIATE_TEST_CASE_P(BiaryOps, MetaParser, ::testing::Values(
    TestData({"+", BinaryOp::add}),
    TestData({"-", BinaryOp::sub}),
    TestData({"*", BinaryOp::mul}),
    TestData({"/", BinaryOp::div}),
    TestData({"==", BinaryOp::equal}),
    TestData({"!=", BinaryOp::noteq}),
    TestData({"<", BinaryOp::less}),
    TestData({">", BinaryOp::greater}),
    TestData({"<=", BinaryOp::lesseq}),
    TestData({">=", BinaryOp::greatereq}),
    TestData({"&&", BinaryOp::boolAnd}),
    TestData({"||", BinaryOp::boolOr})
));
