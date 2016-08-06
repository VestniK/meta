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

#include "utils/types.h"

#include "parser/metalexer.h"

using namespace meta;

TEST(Lexer, separated) {
    const char* input = "testIdentifier1;testIdentifier2,test.qname";
    meta::Lexer lexer;
    lexer.start(input);
    lexer.next();
    EXPECT_EQ(static_cast<utils::string_view>(lexer.currentToken()), "testIdentifier1"sv);
    lexer.next();
    EXPECT_EQ(static_cast<utils::string_view>(lexer.currentToken()), ";"sv);
    lexer.next();
    EXPECT_EQ(static_cast<utils::string_view>(lexer.currentToken()), "testIdentifier2"sv);
    lexer.next();
    EXPECT_EQ(static_cast<utils::string_view>(lexer.currentToken()), ","sv);
    lexer.next();
    EXPECT_EQ(static_cast<utils::string_view>(lexer.currentToken()), "test"sv);
    lexer.next();
    ASSERT_EQ(static_cast<utils::string_view>(lexer.currentToken()), "."sv);
    lexer.next();
    ASSERT_EQ(static_cast<utils::string_view>(lexer.currentToken()), "qname"sv);
}
