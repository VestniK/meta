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

#include "analysers/resolver.h"
#include "analysers/semanticerror.h"
#include "analysers/typechecker.h"

#include "parser/binaryop.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/parse.h"
#include "parser/vardecl.h"

#include "typesystem/typesstore.h"

namespace {

struct NameType
{
    NameType(const std::string name, typesystem::Type::TypeId type): name(name), type(type) {}

    std::string name;
    typesystem::Type::TypeId type;
};
typedef std::vector<NameType> NameTypeList;

struct TestData
{
    TestData(const char *src, const NameTypeList &func, const NameTypeList &vars): src(src), functions(func), vars(vars) {}

    const char *src;
    NameTypeList functions;
    NameTypeList vars;
};

class TypeCheker: public testing::TestWithParam<TestData>
{
public:
};

}

TEST_P(TypeCheker, typeCheck) {
    auto param = GetParam();
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(param.src, strlen(param.src))));
    ASSERT_NO_THROW(analysers::resolve(ast.get()));
    typesystem::TypesStore typestore;
    ASSERT_NO_THROW(analysers::checkTypes(ast.get(), typestore));
    auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), param.functions.size());
    for (size_t pos = 0; pos < functions.size(); ++pos) {
        ASSERT_EQ(functions[pos]->name(), param.functions[pos].name);
        ASSERT_EQ(functions[pos]->type()->typeId(), param.functions[pos].type);
    }

    auto vars = ast->getChildren<meta::VarDecl>(-1);
    ASSERT_EQ(vars.size(), param.vars.size());
    for (size_t pos = 0; pos < vars.size(); ++pos) {
        ASSERT_EQ(vars[pos]->name(), param.vars[pos].name);
        ASSERT_EQ(vars[pos]->type()->typeId(), param.vars[pos].type);
    }

    std::vector<meta::Typed*> exprs;
    ast->walkTopDown<meta::Node>([&exprs](meta::Node *node){auto typed = dynamic_cast<meta::Typed*>(node); if (typed) exprs.push_back(typed); return true;});
    for (auto typed : exprs) {
        ASSERT_NE(typed->type(), nullptr) << "Type not set for node of type " << typeid(*typed).name();
        ASSERT_NE(typed->type()->typeId(), typesystem::Type::Auto) << "Type is incomplete for node of type " << typeid(*typed).name();
    }
}

INSTANTIATE_TEST_CASE_P(typeCheckAndDeduce, TypeCheker, ::testing::Values(
    TestData(
        "package test; int foo() {return 5;} bool bar(int x) {return x < 5;}",
        NameTypeList({NameType("foo", typesystem::Type::Int), NameType("bar", typesystem::Type::Bool)}),
        NameTypeList({NameType("x", typesystem::Type::Int)})
    ),
    TestData(
        "package test; auto foo() {return 5;} auto bar(int x) {return x < 5;}",
        NameTypeList({NameType("foo", typesystem::Type::Int), NameType("bar", typesystem::Type::Bool)}),
        NameTypeList({NameType("x", typesystem::Type::Int)})
    ),
    TestData(
        "package test; bool foo(int x, int y) {auto var = x+y; auto flag = x < y; return flag == x < var;}",
        NameTypeList({NameType("foo", typesystem::Type::Bool)}),
        NameTypeList({NameType("x", typesystem::Type::Int), NameType("y", typesystem::Type::Int), NameType("var", typesystem::Type::Int), NameType("flag", typesystem::Type::Bool)})
    ),
    TestData(
        "package test; bool foo(auto x = 5, auto y = 5 < 2) {return y != x < 5;}",
        NameTypeList({NameType("foo", typesystem::Type::Bool)}),
        NameTypeList({NameType("x", typesystem::Type::Int), NameType("y", typesystem::Type::Bool)})
    ),
    TestData(
        "package test; auto foo() {auto val = 5; return val;}",
        NameTypeList({NameType("foo", typesystem::Type::Int)}),
        NameTypeList({NameType("val", typesystem::Type::Int)})
    )
));

namespace {

class TypeChekerErrors: public testing::TestWithParam<const char *>
{
public:
};

}

TEST_P(TypeChekerErrors, typeErrors) {
    const char *input = GetParam();
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    ASSERT_NO_THROW(analysers::resolve(ast.get()));
    typesystem::TypesStore typestore;
    try {
        analysers::checkTypes(ast.get(), typestore);
        ASSERT_TRUE(false) << "Input code contains type integrity or deduce error which was not found";
    } catch (analysers::SemanticError &err) {
        ASSERT_EQ(err.tokens().linenum(), 2) << err.what() << ": " << std::string(err.tokens());
        ASSERT_EQ(err.tokens().colnum(), 1) << err.what() << ": " << std::string(err.tokens());
    }
}

INSTANTIATE_TEST_CASE_P(inconsistentTypes, TypeChekerErrors, ::testing::Values(
    "package test; int foo(int x) {\nreturn x < 5;}", // wrrong return expr type
    "package test; auto foo(int x) {\nbool b = x; return b;}", // incorret type of init expr
    "package test; auto foo(int x) {bool b; \nb = x; return b;}", // incorret type of assign
    // Deduce loops
    "package test; auto foo() {return bar();} auto bar() {\nreturn foo();}",
    // arythmetic on incompatible
    "package test; auto foo(int x, bool y) {return \nx + y;}",
    "package test; auto foo(int x, bool y) {return \nx - y;}",
    "package test; auto foo(int x, bool y) {return \nx * y;}",
    "package test; auto foo(int x, bool y) {return \nx / y;}",
    // numeric operation on non-numeric
    "package test; auto foo(bool x, bool y) {return \nx + y;}",
    "package test; auto foo(bool x, bool y) {return \nx - y;}",
    "package test; auto foo(bool x, bool y) {return \nx * y;}",
    "package test; auto foo(bool x, bool y) {return \nx / y;}",
    "package test; auto foo(bool x, bool y) {return \nx > y;}",
    "package test; auto foo(bool x, bool y) {return \nx >= y;}",
    "package test; auto foo(bool x, bool y) {return \nx < y;}",
    "package test; auto foo(bool x, bool y) {return \nx <= y;}",
    // Equality on different types
    "package test; auto foo(int x, bool y) {return \nx == y;}",
    "package test; auto foo(int x, bool y) {return \nx != y;}"
));
