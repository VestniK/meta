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

#include <boost/format.hpp>

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
#include "parser/var.h"
#include "parser/vardecl.h"

using namespace meta;

using namespace std::literals;

TEST(MetaParser, imports) {
    const auto input = "package test; import pkg.bar; import pkg.subpkg.bar as bar1; int foo() {return 5;}"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    const auto imports = ast->getChildren<Import>(-1);
    ASSERT_EQ(imports.size(), 2u);
    ASSERT_EQ(imports[0]->name(), "bar");
    ASSERT_EQ(imports[0]->target(), "bar");
    ASSERT_EQ(imports[0]->targetPackage(), "pkg");
    ASSERT_EQ(imports[1]->name(), "bar1");
    ASSERT_EQ(imports[1]->target(), "bar");
    ASSERT_EQ(imports[1]->targetPackage(), "pkg.subpkg");
}

TEST(MetaParser, emptyPackage) {
    const auto input = "package test.test;"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    const auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), 0u);
}

TEST(MetaParser, varTest) {
    const auto input = R"META(
        package test;
        int foo(int x)
        {
            int y = 2*x;
            int z;
            z = x*x;
            return z + y - 3;
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto blocks = ast->getChildren<CodeBlock>(-1);
    ASSERT_EQ(blocks.size(), 1u);
    auto varDeclarations = blocks.front()->getChildren<VarDecl>(-1);
    ASSERT_EQ(varDeclarations.size(), 2u);
    ASSERT_EQ(varDeclarations[0]->typeName(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_TRUE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->typeName(), "int");
    ASSERT_EQ(varDeclarations[1]->name(), "z");
    ASSERT_FALSE(varDeclarations[1]->inited());

    auto assigments = blocks.front()->getChildren<Assigment>(-1);
    ASSERT_EQ(assigments.size(), 1u);
    ASSERT_NE(dynamic_cast<Var*>(assigments[0]->target()), nullptr);
    ASSERT_EQ(dynamic_cast<Var*>(assigments[0]->target())->name(), "z");
}

TEST(MetaParser, assignAsExpr) {
    const auto input = R"META(
        package test;
        int foo(int x)
        {
            int y;
            int z;
            z = (y = x + 1)*x;
            return z + y - 3;
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto blocks = ast->getChildren<CodeBlock>(-1);
    ASSERT_EQ(blocks.size(), 1u);
    auto varDeclarations = blocks.front()->getChildren<VarDecl>(-1);
    ASSERT_EQ(varDeclarations.size(), 2u);
    ASSERT_EQ(varDeclarations[0]->typeName(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_FALSE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->typeName(), "int");
    ASSERT_EQ(varDeclarations[1]->name(), "z");
    ASSERT_FALSE(varDeclarations[1]->inited());

    auto assigments = blocks.front()->getChildren<Assigment>(-1);
    ASSERT_EQ(assigments.size(), 2u);
    ASSERT_NE(dynamic_cast<Var*>(assigments[0]->target()), nullptr);
    ASSERT_EQ(dynamic_cast<Var*>(assigments[0]->target())->name(), "z");
    ASSERT_NE(dynamic_cast<Var*>(assigments[1]->target()), nullptr);
    ASSERT_EQ(dynamic_cast<Var*>(assigments[1]->target())->name(), "y");
}

TEST(MetaParser, ifStatement) {
    const auto input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                foo1(x);
            foo2(x);
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1u);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 2u);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1u);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
}

TEST(MetaParser, ifWithEmptyStatement) {
    const auto input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                ;
            foo1(x);
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1u);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 1u);
    ASSERT_EQ(calls[0]->functionName(), "foo1");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
}

TEST(MetaParser, ifElseStatement) {
    const auto input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                foo1(x);
            else
                foo2(x);
            foo3(x);
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1u);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 3u);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");
    ASSERT_EQ(calls[2]->functionName(), "foo3");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_NE(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1u);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
    ASSERT_EQ(ifs[0]->elseBlock()->getChildren<Call>(-1).size(), 1u);
    ASSERT_EQ(ifs[0]->elseBlock()->getChildren<Call>(-1)[0], calls[1]);
}

TEST(MetaParser, ifElseEmptyStatement) {
    const auto input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                foo1(x);
            else
                ;
            foo2(x);
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1u);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 2u);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1u);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
}

TEST(MetaParser, ifElseBothEmptyStatements) {
    const auto input = R"META(
        package test;

        void foo0(int x)
        {
            if (x < 0)
                ;
            else
                ;
            foo1(x);
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1u);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 1u);
    ASSERT_EQ(calls[0]->functionName(), "foo1");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
}

TEST(MetaParser, ifBlockStatement) {
    const auto input = R"META(
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
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1u);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 2u);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_EQ(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1u);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
}

TEST(MetaParser, ifElseBlockStatement) {
    const auto input = R"META(
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
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto ifs = ast->getChildren<If>(-1);
    ASSERT_EQ(ifs.size(), 1u);
    auto calls = ast->getChildren<Call>(-1);
    ASSERT_EQ(calls.size(), 3u);
    ASSERT_EQ(calls[0]->functionName(), "foo1");
    ASSERT_EQ(calls[1]->functionName(), "foo2");
    ASSERT_EQ(calls[2]->functionName(), "foo3");

    ASSERT_NE(ifs[0]->condition(), nullptr);
    ASSERT_NE(ifs[0]->thenBlock(), nullptr);
    ASSERT_NE(ifs[0]->elseBlock(), nullptr);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1).size(), 1u);
    ASSERT_EQ(ifs[0]->thenBlock()->getChildren<Call>(-1)[0], calls[0]);
    ASSERT_EQ(ifs[0]->elseBlock()->getChildren<Call>(-1).size(), 1u);
    ASSERT_EQ(ifs[0]->elseBlock()->getChildren<Call>(-1)[0], calls[1]);
}

TEST(MetaParser, multipleFiles) {
    const auto src1 = "package test; int foo() {return 0;}"s;
    const auto src2 = "package test; bool bar() {return false;}"s;

    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("src1", src1));
    ASSERT_NO_THROW(parser.parse("src2", src2));

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
    ASSERT_EQ(foo->sourceLocation(), "src1");
    ASSERT_NE(bar, nullptr);
    ASSERT_EQ(bar->sourceLocation(), "src2");
}

namespace {

template<size_t N>
inline std::vector<char> str2buf(const char (&str)[N])
{
    return std::vector<char>(str, str + N - 1); // ignore auto assigned zero terminator
}

}

TEST(Parser, stringLiteral) {
    const auto input = R"META(
        package test;
        string foo() {
            return "Hello World";
        }
        string escaped() {
            return "\t \n \r \a \b \f \\ \" \0";
        }
    )META"s;

    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));

    auto strs = parser.ast()->getChildren<StrLiteral>(-1);
    ASSERT_EQ(strs.size(), 2u);
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
    const std::string input = str(boost::format("package test; int foo(int x, int y) {return x %s y;}")%data.opStr);
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto binops = ast->getChildren<BinaryOp>(-1);
    ASSERT_EQ(binops.size(), 1u);
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
