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

#include "analysers/typechecker.h"
#include "analysers/resolver.h"

#include "parser/binaryop.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/parse.h"
#include "parser/vardecl.h"

#include "typesystem/typesstore.h"

TEST(TypeCheker, simpleTypeCheck) {
    const char *input = "package test; int foo() {return 5;} bool bar(int x) {return x < 5;}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    ASSERT_NO_THROW(analysers::resolve(ast.get()));
    typesystem::TypesStore typestore;
    ASSERT_NO_THROW(analysers::checkTypes(ast.get(), typestore));
    auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 2);
    ASSERT_EQ(functions[0]->name(), std::string("foo"));
    ASSERT_EQ(functions[0]->type()->typeId(), typesystem::Type::Int);
    ASSERT_EQ(functions[1]->name(), std::string("bar"));
    ASSERT_EQ(functions[1]->type()->typeId(), typesystem::Type::Bool);

    std::vector<meta::Typed*> exprs;
    ast->walkTopDown<meta::Node>([&exprs](meta::Node *node){auto typed = dynamic_cast<meta::Typed*>(node); if (typed) exprs.push_back(typed); return true;});
    for (auto typed : exprs) {
        ASSERT_NE(typed->type(), nullptr) << "Type not set for node of type " << typeid(*typed).name();
        ASSERT_NE(typed->type()->typeId(), typesystem::Type::Auto) << "Type is incomplete for node of type " << typeid(*typed).name();
    }
}

TEST(TypeCheker, simpleRetTypeDeduce) {
    const char *input = "package test; auto foo() {return 5;} auto bar(int x) {return x < 5;}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    ASSERT_NO_THROW(analysers::resolve(ast.get()));
    typesystem::TypesStore typestore;
    ASSERT_NO_THROW(analysers::checkTypes(ast.get(), typestore));
    auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 2);
    ASSERT_EQ(functions[0]->name(), std::string("foo"));
    ASSERT_EQ(functions[0]->type()->typeId(), typesystem::Type::Int);
    ASSERT_EQ(functions[1]->name(), std::string("bar"));
    ASSERT_EQ(functions[1]->type()->typeId(), typesystem::Type::Bool);

    std::vector<meta::Typed*> exprs;
    ast->walkTopDown<meta::Node>([&exprs](meta::Node *node){auto typed = dynamic_cast<meta::Typed*>(node); if (typed) exprs.push_back(typed); return true;});
    for (auto typed : exprs) {
        ASSERT_NE(typed->type(), nullptr) << "Type not set for node of type " << typeid(*typed).name();
        ASSERT_NE(typed->type()->typeId(), typesystem::Type::Auto) << "Type is incomplete for node of type " << typeid(*typed).name();
    }
}

TEST(TypeCheker, doubleDeduce) {
    const char *input = "package test; auto foo() {auto val = 5; return val;}";
    std::unique_ptr<meta::AST> ast;
    ASSERT_NO_THROW(ast = std::unique_ptr<meta::AST>(parse(input, strlen(input))));
    ASSERT_NO_THROW(analysers::resolve(ast.get()));
    typesystem::TypesStore typestore;
    ASSERT_NO_THROW(analysers::checkTypes(ast.get(), typestore));
    auto functions = ast->getChildren<meta::Function>();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), std::string("foo"));
    ASSERT_EQ(functions[0]->type()->typeId(), typesystem::Type::Int);
    auto vars = ast->getChildren<meta::VarDecl>(-1);
    ASSERT_EQ(vars.size(), 1);
    ASSERT_EQ(vars[0]->name(), std::string("val"));
    ASSERT_EQ(vars[0]->type()->typeId(), typesystem::Type::Int);

    std::vector<meta::Typed*> exprs;
    ast->walkTopDown<meta::Node>([&exprs](meta::Node *node){auto typed = dynamic_cast<meta::Typed*>(node); if (typed) exprs.push_back(typed); return true;});
    for (auto typed : exprs) {
        ASSERT_NE(typed->type(), nullptr) << "Type not set for node of type " << typeid(*typed).name();
        ASSERT_NE(typed->type()->typeId(), typesystem::Type::Auto) << "Type is incomplete for node of type " << typeid(*typed).name();
    }
}
