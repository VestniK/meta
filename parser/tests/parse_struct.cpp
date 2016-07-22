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
#include "parser/assigment.h"
#include "parser/metaparser.h"
#include "parser/struct.h"
#include "parser/vardecl.h"

using namespace meta;

using namespace std::literals;

TEST(StructParsing, simpleStruct) {
     const auto input = R"META(
        package test;

        struct Point {
            int x;
            int y;
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto structs = ast->getChildren<Struct>();
    ASSERT_EQ(structs.size(), 1u);
    EXPECT_EQ(structs[0]->name(), "Point");
    auto members = structs[0]->getChildren<VarDecl>();
    ASSERT_EQ(members.size(), 2u);
    EXPECT_EQ(members[0]->name(), "x");
    EXPECT_EQ(members[1]->name(), "y");
}

TEST(StructParsing, defaultValsOnMembers) {
     const auto input = R"META(
        package test;

        struct Point {
            int x = 0;
            int y = 0;
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto structs = ast->getChildren<Struct>();
    ASSERT_EQ(structs.size(), 1u);
    EXPECT_EQ(structs[0]->name(), "Point");
    auto members = structs[0]->getChildren<VarDecl>();
    ASSERT_EQ(members.size(), 2u);
    EXPECT_EQ(members[0]->name(), "x");
    EXPECT_EQ(static_cast<utils::string_view>(members[0]->initExpr()->tokens()), "0");
    EXPECT_EQ(members[1]->name(), "y");
    EXPECT_EQ(static_cast<utils::string_view>(members[1]->initExpr()->tokens()), "0");
}

TEST(StructParsing, declareStructVar) {
     const auto input = R"META(
        package test;

        struct Point {
            int x = 0;
            int y = 0;
        }

        void foo() {
            Point pt;
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto vars = ast->getChildren<VarDecl>(infinitDepth);
    ASSERT_EQ(vars.size(), 3u);
    EXPECT_EQ(vars[2]->name(), "pt");
    EXPECT_EQ(vars[2]->typeName(), "Point");
}

TEST(StructParsing, memberAccess) {
     const auto input = R"META(
        package test;

        struct Point {
            int x = 0;
            int y = 0;
        }

        void foo() {
            Point pt;
            pt.y = 5;
            pt.x = pt.y + 7;
        }
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta", input));
    auto ast = parser.ast();
    auto assigments = ast->getChildren<Assigment>(infinitDepth);
    ASSERT_EQ(assigments.size(), 2u);
//    EXPECT_EQ(assigments[2]->, "pt");
//    EXPECT_EQ(assigments[3]->typeName(), "Point");
}
