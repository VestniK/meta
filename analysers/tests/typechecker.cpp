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

#include "utils/testtools.h"

#include "typesystem/typesstore.h"

#include "parser/binaryop.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/vardecl.h"

#include "analysers/actions.h"
#include "analysers/declconflicts.h" // TODO elliminate necessity of this header (SourceInfo only)
#include "analysers/resolver.h"
#include "analysers/semanticerror.h"
#include "analysers/typechecker.h"

namespace meta::analysers::tests {
namespace {

struct NameType {
    NameType(const std::string name, typesystem::Type::TypeId type): name(name), type(type) {}

    std::string name;
    typesystem::Type::TypeId type;
};
using NameTypeList = std::vector<NameType>;

struct TestData {
    utils::SourceFile src;
    NameTypeList functions;
    NameTypeList vars;
};

class TypeCheker: public testing::TestWithParam<TestData> {};

TEST_P(TypeCheker, typeCheck) {
    auto param = GetParam();
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, param.src);
    auto ast = parser.ast();
    ASSERT_ANALYSE(resolve(ast, act.dictionary()));
    typesystem::TypesStore typestore;
    ASSERT_ANALYSE(checkTypes(ast, typestore));
    auto functions = ast->getChildren<Function>();
    ASSERT_EQ(functions.size(), param.functions.size());
    for (size_t pos = 0; pos < functions.size(); ++pos) {
        ASSERT_EQ(functions[pos]->name(), param.functions[pos].name);
        ASSERT_EQ(functions[pos]->type()->typeId(), param.functions[pos].type);
    }

    auto vars = ast->getChildren<VarDecl>(-1);
    ASSERT_EQ(vars.size(), param.vars.size());
    for (size_t pos = 0; pos < vars.size(); ++pos) {
        ASSERT_EQ(vars[pos]->name(), param.vars[pos].name);
        ASSERT_EQ(vars[pos]->type()->typeId(), param.vars[pos].type);
    }

    std::vector<Typed*> exprs;
    walk<Node, TopDown>(*ast, [&exprs](Node *node){
        auto typed = dynamic_cast<Typed*>(node);
        if (typed)
            exprs.push_back(typed);
        return true;
    });
    for (auto typed : exprs) {
        ASSERT_NE(typed->type(), nullptr) << "Type not set for node of type " << typeid(*typed).name();
        ASSERT_NE(typed->type()->typeId(), typesystem::Type::Auto) << "Type is incomplete for node of type " << typeid(*typed).name();
    }
}

INSTANTIATE_TEST_CASE_P(typeCheckAndDeduce, TypeCheker, ::testing::Values(
    TestData{
        "package test; int foo() {return 5;} bool bar(int x) {return x < 5;}",
        NameTypeList({NameType("foo", typesystem::Type::Int), NameType("bar", typesystem::Type::Bool)}),
        NameTypeList({NameType("x", typesystem::Type::Int)})
    },
    TestData{
        "package test; auto foo() {return 5;} auto bar(int x) {return x < 5;}",
        NameTypeList({NameType("foo", typesystem::Type::Int), NameType("bar", typesystem::Type::Bool)}),
        NameTypeList({NameType("x", typesystem::Type::Int)})
    },
    TestData{
        "package test; bool foo(int x, int y) {auto var = x+y; auto flag = x < y; return flag == x < var;}",
        NameTypeList({NameType("foo", typesystem::Type::Bool)}),
        NameTypeList({NameType("x", typesystem::Type::Int), NameType("y", typesystem::Type::Int), NameType("var", typesystem::Type::Int), NameType("flag", typesystem::Type::Bool)})
    },
    TestData{
        "package test; bool foo(auto x = 5, auto y = !(5 < 2)) {return y != x < 5;}",
        NameTypeList({NameType("foo", typesystem::Type::Bool)}),
        NameTypeList({NameType("x", typesystem::Type::Int), NameType("y", typesystem::Type::Bool)})
    },
    TestData{
        "package test; auto foo() {auto val = 5; return val;}",
        NameTypeList({NameType("foo", typesystem::Type::Int)}),
        NameTypeList({NameType("val", typesystem::Type::Int)})
    },
    TestData{
        "package test; auto foo() {auto val = true; return val;}",
        NameTypeList({NameType("foo", typesystem::Type::Bool)}),
        NameTypeList({NameType("val", typesystem::Type::Bool)})
    },
    TestData{
        "package test; auto foo() {auto val = false; return val || true;}",
        NameTypeList({NameType("foo", typesystem::Type::Bool)}),
        NameTypeList({NameType("val", typesystem::Type::Bool)})
    },
    TestData{
        "package test; auto foo() {auto val = false; if (val) val = false; return val && true;}",
         NameTypeList({NameType("foo", typesystem::Type::Bool)}),
         NameTypeList({NameType("val", typesystem::Type::Bool)})
    },
    TestData{
        "package test; auto foo() {auto val = false; if (val) {int x = 5; return x > 2;} return val && true;}",
        NameTypeList({NameType("foo", typesystem::Type::Bool)}),
        NameTypeList({NameType("val", typesystem::Type::Bool), NameType("x", typesystem::Type::Int)})
    },
    TestData{
        "package test; auto foo() {auto val = false; if (val) val = false; else return val || true; return val && true;}",
         NameTypeList({NameType("foo", typesystem::Type::Bool)}),
         NameTypeList({NameType("val", typesystem::Type::Bool)})
    },
    TestData{
        "package test; auto foo() {auto val = false; if (val) {int x = 5; return x > 2;} else {int y = 7; val = val && y < 5;} return val && true;}",
        NameTypeList({NameType("foo", typesystem::Type::Bool)}),
        NameTypeList({NameType("val", typesystem::Type::Bool), NameType("x", typesystem::Type::Int), NameType("y", typesystem::Type::Int)})
    },
    TestData{
        R"META(package test; auto foo() {auto var = "Hello"; return var;})META",
        NameTypeList({NameType("foo", typesystem::Type::String)}),
        NameTypeList({NameType("var", typesystem::Type::String)})
    },
    TestData{
        R"META(package test; string foo() {auto var = "Hello"; return var;})META",
        NameTypeList({NameType("foo", typesystem::Type::String)}),
        NameTypeList({NameType("var", typesystem::Type::String)})
    },
    TestData{
        R"META(package test; auto foo() {string var = "Hello"; return var;})META",
        NameTypeList({NameType("foo", typesystem::Type::String)}),
        NameTypeList({NameType("var", typesystem::Type::String)})
    },
    TestData{
        R"META(package test; string foo() {string var = "Hello"; return var;})META",
        NameTypeList({NameType("foo", typesystem::Type::String)}),
        NameTypeList({NameType("var", typesystem::Type::String)})
    }
));


class TypeChekerErrors: public utils::ErrorTest {};

TEST_P(TypeChekerErrors, typeErrors) {
    const auto& param = GetParam();
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, param.input);
    auto ast = parser.ast();
    ASSERT_ANALYSE(resolve(ast, act.dictionary()));
    typesystem::TypesStore typestore;
    try {
        checkTypes(ast, typestore);
        FAIL() << "Error was not detected: " << param.errMsg;
    } catch (const SemanticError &err) {
        EXPECT_EQ(err.what(), param.errMsg) << SourceInfo(err) << ": " << err.what();
    }
}

INSTANTIATE_TEST_CASE_P(inconsistentTypes, TypeChekerErrors, ::testing::Values(
    utils::ErrorTestData{
        .input = R"META(
            package test;

            int foo(int x) {
                return x < 5;
            }
        )META",
        .errMsg =
R"(Function 'protFoo' from the package 'test.lib' has no overloads visible from the current package 'test'
notice: lib.meta:39:5: Function 'test.lib.protFoo(int)' is protected)"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x) {
                bool b = x;
                return b;
            }
        )META",
        .errMsg =
R"(Function 'protFoo' from the package 'test.lib' has no overloads visible from the current package 'test'
notice: lib.meta:39:5: Function 'test.lib.protFoo(int)' is protected)"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x) {
                bool b;
                b = x;
                return b;
            }
        )META",
        .errMsg =
R"(Function 'protFoo' from the package 'test.lib' has no overloads visible from the current package 'test'
notice: lib.meta:39:5: Function 'test.lib.protFoo(int)' is protected)"
    },
    // Deduce loops
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo() {
                return bar();
            }
            auto bar() {
                return foo();
            }
        )META",
        .errMsg = ""
    },
    // arythmetic on incompatible
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x, bool y) {
                return x + y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x, bool y) {
                return x - y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x, bool y) {
                return x * y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x, bool y) {
                return x / y;
            }
        )META",
        .errMsg = ""
    },
    // numeric operation on non-numeric
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(bool x, bool y) {
                return x + y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(bool x, bool y) {
                return x - y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(bool x, bool y) {
                return x * y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(bool x, bool y) {
                return x / y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(bool x, bool y) {
                return x > y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(bool x, bool y) {
                return x >= y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(bool x, bool y) {
                return x < y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(bool x, bool y) {
                return x <= y;
            }
        )META",
        .errMsg = ""
    },
    // Equality on different types
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x, bool y) {
                return x == y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x, bool y) {
                return x != y;
            }
        )META",
        .errMsg = ""
    },
    // Bool operations on nonbool
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x) {
                return !x;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x, int y) {
                return x && y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            auto foo(int x, int y) {
                return x || y;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            int foo(int x) {
                if (x)
                    return 1;
                return 0;
            }
        )META",
        .errMsg = ""
    },
    // Types are checked inside if branches
    utils::ErrorTestData{
        .input = R"META(
            package test;

            bool foo(int x, int y) {
                if (true)
                    return x && y;
                return false;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            bool foo(int x, int y) {
                if (true) {
                    auto z = x+y;
                    return z && y;
                }
                return false;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            bool foo(int x, int y) {
                if (true)
                    return true;
                else
                    return x && y;
                return false;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            bool foo(int x, int y) {
                if (true) {
                    auto res = x < y;
                    return res;
                } else {
                    return x && y;
                }
                return false;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            bool foo(int x, int y) {
                if (true)
                    return x && y;
                else
                    return true;
                return false;
            }
        )META",
        .errMsg = ""
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            bool foo(int x, int y) {
                if (true) {
                    auto z = x+y; return z && y;
                } else {
                    auto res = x != y;
                    return res;
                }
                return false;
            }
        )META",
        .errMsg = ""
    }
));


} // anonymous namespace
} // namespace meta::analysers::tests
