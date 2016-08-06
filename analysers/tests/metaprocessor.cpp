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

#include <gtest/gtest.h>

#include "parser/function.h"
#include "parser/metaparser.h"

#include "analysers/actions.h"
#include "analysers/metaprocessor.h"

using namespace meta;
using namespace meta::analysers;

using namespace std::literals;

TEST(MetaProcessor, attribute)
{
    const auto input = R"META(
        package test;

        @entrypoint
        int foo() {return 0;}

        int bar() {return 1;}
    )META"s;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_NO_THROW(parser.parse("test.meta",input));
    auto ast = parser.ast();
    ASSERT_NO_THROW(processMeta(ast));
    auto functions = ast->getChildren<Function>(-1);
    ASSERT_EQ(functions.size(), 2u);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_TRUE(functions[0]->flags() & FuncFlags::entrypoint);

    ASSERT_EQ(functions[1]->name(), "bar");
    ASSERT_FALSE(functions[1]->flags() & FuncFlags::entrypoint);
}
