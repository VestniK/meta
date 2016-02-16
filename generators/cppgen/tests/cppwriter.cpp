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

#include <sstream>

#include <gtest/gtest.h>

#include "typesystem/typesstore.h"

#include "generators/cppgen/cppwriter.h"

using namespace meta;
using namespace meta::generators::cppgen;
using namespace std::string_literals;

TEST(CppWriterTests, nestedNamespace) {
    std::ostringstream out;
    {
        CppWriter writer(out);
        writer.setPackage("a.b.c.d");
        writer.setPackage("a.b.e.f");
        writer.setPackage("a.g.h");
        writer.setPackage("p.q.l");
        writer.setPackage("f");
    }
    const auto expected =
R"(namespace a {
namespace b {
namespace c {
namespace d {
}
}
namespace e {
namespace f {
}
}
}
namespace g {
namespace h {
}
}
}
namespace p {
namespace q {
namespace l {
}
}
}
namespace f {
}
)"s
    ;
    ASSERT_EQ(expected, out.str());
}

TEST(CppWriterTests, emptyNamespace) {
    const char* src = "package test; extern int foo();";
    Parser parser;
    parser.parse(src);
    auto* foo = parser.ast()->getChildren<Function>()[0];
    typesystem::TypesStore typestore;
    foo->setType(typestore.getPrimitive(typesystem::Type::Int));
    std::ostringstream out;
    {
        CppWriter writer(out);
        writer.setPackage("a.b.c.d");
        writer.setPackage("");
        writer.forwardDeclare(foo);
    }
    const auto expected =
R"(namespace a {
namespace b {
namespace c {
namespace d {
}
}
}
}
int foo();
)"s
    ;
    ASSERT_EQ(expected, out.str());
}
