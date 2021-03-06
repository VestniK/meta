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

#include <gtest/gtest.h>

#include "utils/range.h"
#include "utils/string.h"
#include "utils/testtools.h"

#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/struct.h"

#include "analysers/actions.h"
#include "analysers/metaprocessor.h"
#include "analysers/semanticerror.h"

namespace meta::analysers::tests::actions {
namespace {

const auto input = R"META(
    package test;

    void privateByDefault() {return;}
    struct PrivateByDefault {int x = 0;}

    public void publicExplicitly() {return;}
    public struct PublicExplicitly {int x = 0;}

    protected:

    void protectedByUpdatedDefault() {return;}
    struct ProtectedByUpdatedDefault {int x = 0;}

    export void exportExplicitly() {return;}
    export struct ExportExplicitly {int x;}

    public:

    void publicByDefault() {return;}
    struct PublicByDefault {int x;}

    extern void externExplicitly();
    extern struct ExternExplicitly {int x;}

    extern:

    void externByDefault();
    struct ExternByDefault {int x;}

    public void publicExplicitly(bool b) {return;}
    public struct PublicExplicitly2 {int x;}
)META"_fake_src;

TEST(ActionsTest, funcVisibility) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();

    auto functions = ast->getChildren<Function>(-1);
    ASSERT_EQ(functions.size(), 8u);

    for (Function* func: functions) {
        EXPECT_NE(func->visibility(), Visibility::Default) << func->name();
        if (utils::starts_with(func->name(), "export"))
            EXPECT_EQ(func->visibility(), Visibility::Export) << func->name();
        else if (utils::starts_with(func->name(), "extern"))
            EXPECT_EQ(func->visibility(), Visibility::Extern) << func->name();
        else if (utils::starts_with(func->name(), "public"))
            EXPECT_EQ(func->visibility(), Visibility::Public) << func->name();
        else if (utils::starts_with(func->name(), "protected"))
            EXPECT_EQ(func->visibility(), Visibility::Protected) << func->name();
        else if (utils::starts_with(func->name(), "private"))
            EXPECT_EQ(func->visibility(), Visibility::Private) << func->name();
        else
            FAIL() << func->name();
    }
}

TEST(ActionsTest, structVisibility) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();

    auto structs = ast->getChildren<Struct>(-1);
    ASSERT_EQ(structs.size(), 8u);

    for (Struct* strct: structs) {
        EXPECT_NE(strct->visibility(), Visibility::Default) << strct->name();
        if (utils::starts_with(strct->name(), "Export"))
            EXPECT_EQ(strct->visibility(), Visibility::Export) << strct->name();
        else if (utils::starts_with(strct->name(), "Extern"))
            EXPECT_EQ(strct->visibility(), Visibility::Extern) << strct->name();
        else if (utils::starts_with(strct->name(), "Public"))
            EXPECT_EQ(strct->visibility(), Visibility::Public) << strct->name();
        else if (utils::starts_with(strct->name(), "Protected"))
            EXPECT_EQ(strct->visibility(), Visibility::Protected) << strct->name();
        else if (utils::starts_with(strct->name(), "Private"))
            EXPECT_EQ(strct->visibility(), Visibility::Private) << strct->name();
        else
            FAIL() << strct->name();
    }
}

TEST(ActionsTest, funcPackages) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();

    auto functions = ast->getChildren<Function>(-1);
    ASSERT_EQ(functions.size(), 8u);

    for (Function* func: functions)
        EXPECT_EQ(func->package(), "test") << func->name();
}

TEST(ActionsTest, structPackages) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();

    auto structs = ast->getChildren<Struct>(-1);
    ASSERT_EQ(structs.size(), 8u);

    for (Struct* strct: structs)
        EXPECT_EQ(strct->package(), "test") << strct->name();
}

TEST(ActionsTest, funcDict) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_EQ(act.dictionary().size(), 1u);
    const auto& kv = *(act.dictionary().begin());
    EXPECT_EQ(kv.first, "test");
    EXPECT_EQ(kv.second.functions.size(), 8u);
}

TEST(ActionsTest, structDict) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_EQ(act.dictionary().size(), 1u);
    const auto& kv = *(act.dictionary().begin());
    EXPECT_EQ(kv.first, "test");
    EXPECT_EQ(kv.second.structs.size(), 8u);
}

TEST(ActionsTest, funcOverloads) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    auto funcs = utils::slice(act.dictionary()["test"].functions.equal_range("publicExplicitly"));
    EXPECT_EQ(std::distance(funcs.begin(), funcs.end()), 2);
    for (auto* func: funcs)
        EXPECT_EQ(func->name(), "publicExplicitly");
}

class ActionsTest: public utils::ErrorTest {};

utils::ErrorTestData errorTests[] = {
    { // Struct with struct
        .input = R"META(
            package test;

            struct Point {int x; int y;}

            /*conflict*/ struct Point {int x; int y; int z;}
        )META"_fake_src,
        .errMsg =
            "Struct 'test.Point' conflicts with other declarations.\n"
            "test.meta:4:13: notice: Struct 'test.Point'"
    },
    { // Struct with function
        .input = R"META(
            package test;

            void foo() {return;}

            /*conflict*/ struct foo {int x = 0;}
        )META"_fake_src,
        .errMsg =
            "Struct 'test.foo' conflicts with other declarations.\n"
            "test.meta:4:13: notice: Function 'test.foo()'"
    },
    { // Struct with overloaded function
        .input = R"META(
            package test;
            void foo() {return;}
            void foo(int x) {return;}

            /*conflict*/ struct foo {int x = 0;}
        )META"_fake_src,
        .errMsg =
            "Struct 'test.foo' conflicts with other declarations.\n"
            "test.meta:3:13: notice: Function 'test.foo()'\n"
            "test.meta:4:13: notice: Function 'test.foo(int)'"
    },
    { // Function with struct
        .input = R"META(
            package test;

            struct Point {int x; int y;}

            /*conflict*/ Point Point(int x, int y) {Point res; res.x= x; res.y = y; return res;}
        )META"_fake_src,
        .errMsg =
            "Function 'test.Point(int, int)' conflicts with other declarations.\n"
            "test.meta:4:13: notice: Struct 'test.Point'"
    }
};
INSTANTIATE_TEST_CASE_P(DeclConflincts, ActionsTest, ::testing::ValuesIn(errorTests));

TEST_P(ActionsTest, conflictsTest) {
    const auto& param = GetParam();
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    try {
        parser.parse(param.input);
        FAIL() << "Failed to detect declaration conflict";
    } catch (const SemanticError& err) {
        EXPECT_EQ(param.errMsg, err.what()) << err.what();
        EXPECT_EQ(err.tokens().linenum(), 6);
        EXPECT_EQ(err.tokens().colnum(), 26);
    }
}

} // anonymous namespace
} // namespace meta::analysers
