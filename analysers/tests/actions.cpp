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

#include "utils/string.h"
#include "utils/testtools.h"

#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/struct.h"

#include "analysers/actions.h"
#include "analysers/metaprocessor.h"

using namespace meta;
using namespace meta::analysers;

using namespace std::literals;

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
    struct externByDefault {int x;}

    public void publicExplicitly() {return;}
    public struct PublicExplicitly2 {int x;}
)META"s;
}

TEST(Actions, funcVisibility) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, "test.meta", input);
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
    }
}

TEST(Actions, structVisibility) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, "test.meta", input);
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
    }
}

TEST(Actions, funcPackages) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, "test.meta", input);
    auto ast = parser.ast();

    auto functions = ast->getChildren<Function>(-1);
    ASSERT_EQ(functions.size(), 8u);

    for (Function* func: functions)
        EXPECT_EQ(func->package(), "test") << func->name();
}

TEST(Actions, structPackages) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, "test.meta", input);
    auto ast = parser.ast();

    auto structs = ast->getChildren<Struct>(-1);
    ASSERT_EQ(structs.size(), 8u);

    for (Struct* strct: structs)
        EXPECT_EQ(strct->package(), "test") << strct->name();
}

TEST(Actions, funcDict) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, "test.meta", input);
    ASSERT_EQ(act.dictionary().size(), 1u);
    const auto& kv = *(act.dictionary().begin());
    EXPECT_EQ(kv.first, "test");
    EXPECT_EQ(kv.second.functions.size(), 8u);
    for (const auto& fkv: kv.second.functions)
        EXPECT_EQ(fkv.first, fkv.second->name());
}

TEST(Actions, structDict) {
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, "test.meta", input);
    ASSERT_EQ(act.dictionary().size(), 1u);
    const auto& kv = *(act.dictionary().begin());
    EXPECT_EQ(kv.first, "test");
    EXPECT_EQ(kv.second.structs.size(), 8u);
    for (const auto& skv: kv.second.structs)
        EXPECT_EQ(skv.first, skv.second->name());
}
