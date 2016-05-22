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
#include <string>

#include <gtest/gtest.h>

#include "parser/actions.h"
#include "parser/annotation.h"
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
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    auto structs = ast->getChildren<Struct>();
    ASSERT_EQ(structs.size(), 1u);
    EXPECT_EQ(structs[0]->name(), "Point");
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
    ASSERT_NO_THROW(parser.parse(input));
    auto ast = parser.ast();
    auto structs = ast->getChildren<Struct>();
    ASSERT_EQ(structs.size(), 1u);
    EXPECT_EQ(structs[0]->name(), "Point");
}
