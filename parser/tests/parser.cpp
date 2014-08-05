#include <cassert>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "parser/arg.h"
#include "parser/assigment.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/package.h"
#include "parser/vardecl.h"

TEST(Parser, zeroParamFunc) {
    const char *input = "package test; int foo() {return 5;}";
    meta::Parser parser;
    std::shared_ptr<meta::Package> root(nullptr);
    ASSERT_NO_THROW(root = std::dynamic_pointer_cast<meta::Package>(parser.parse(input)));
    ASSERT_EQ(root->name(), "test");
    const auto functions = root->functions();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    ASSERT_EQ(functions[0]->args().size(), 0);
}

TEST(Parser, oneParamFunc) {
    const char *input = "package test; int foo(int x) {return 5*x*x - 2*x + 3;}";
    meta::Parser parser;
    std::shared_ptr<meta::Package> root(nullptr);
    ASSERT_NO_THROW(root = std::dynamic_pointer_cast<meta::Package>(parser.parse(input)));
    ASSERT_EQ(root->name(), "test");
    const auto functions = root->functions();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 1);
    ASSERT_EQ(args[0]->type(), "int");
    ASSERT_EQ(args[0]->name(), "x");
}

TEST(Parser, twoParamFunc) {
    const char *input = "package test; int foo(int x, int y) {return 5*x + 6/y;}";
    meta::Parser parser;
    std::shared_ptr<meta::Package> root(nullptr);
    ASSERT_NO_THROW(root = std::dynamic_pointer_cast<meta::Package>(parser.parse(input)));
    ASSERT_EQ(root->name(), "test");
    const auto functions = root->functions();
    ASSERT_EQ(functions.size(), 1);
    ASSERT_EQ(functions[0]->name(), "foo");
    const auto args = functions[0]->args();
    ASSERT_EQ(args.size(), 2);
    ASSERT_EQ(args[0]->type(), "int");
    ASSERT_EQ(args[0]->name(), "x");
    ASSERT_EQ(args[1]->type(), "int");
    ASSERT_EQ(args[1]->name(), "y");
}

TEST(Parser, twoFunc) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int x) {return x/5;}";
    meta::Parser parser;
    std::shared_ptr<meta::Package> root(nullptr);
    ASSERT_NO_THROW(root = std::dynamic_pointer_cast<meta::Package>(parser.parse(input)));
    ASSERT_EQ(root->name(), "test");
    const auto functions = root->functions();
    ASSERT_EQ(functions.size(), 2);

    ASSERT_EQ(functions[0]->name(), "foo");
    const auto args1 = functions[0]->args();
    ASSERT_EQ(args1.size(), 1);
    ASSERT_EQ(args1[0]->type(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(functions[1]->name(), "bar");
    const auto args2 = functions[1]->args();
    ASSERT_EQ(args2.size(), 1);
    ASSERT_EQ(args2[0]->type(), "int");
    ASSERT_EQ(args2[0]->name(), "x");
}

TEST(Parser, funcCall) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int y) {return 5*foo(y/5);}";
    meta::Parser parser;
    std::shared_ptr<meta::Package> root(nullptr);
    ASSERT_NO_THROW(root = std::dynamic_pointer_cast<meta::Package>(parser.parse(input)));
    ASSERT_EQ(root->name(), "test");
    const auto functions = root->functions();
    ASSERT_EQ(functions.size(), 2);

    ASSERT_EQ(functions[0]->name(), "foo");
    const auto args1 = functions[0]->args();
    ASSERT_EQ(args1.size(), 1);
    ASSERT_EQ(args1[0]->type(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(functions[1]->name(), "bar");
    const auto args2 = functions[1]->args();
    ASSERT_EQ(args2.size(), 1);
    ASSERT_EQ(args2[0]->type(), "int");
    ASSERT_EQ(args2[0]->name(), "y");
}

TEST(Parser, emptyPackage) {
    const char *input = "package test.test;";
    meta::Parser parser;
    std::shared_ptr<meta::Package> root(nullptr);
    ASSERT_NO_THROW(root = std::dynamic_pointer_cast<meta::Package>(parser.parse(input)));
    ASSERT_EQ(root->name(), "test.test");
    ASSERT_EQ(root->functions().size(), 0);
}

TEST(Parser, funcRetType) {
    const char *input = "package test; int iFoo() {return 0;}\ndouble dFoo() {return 0;}";
    meta::Parser parser;
    std::shared_ptr<meta::Package> root(nullptr);
    ASSERT_NO_THROW(root = std::dynamic_pointer_cast<meta::Package>(parser.parse(input)));
    ASSERT_EQ(root->name(), "test");
    const auto functions = root->functions();
    ASSERT_EQ(functions.size(), 2);
    ASSERT_EQ(functions[0]->name(), "iFoo");
    ASSERT_EQ(functions[0]->retType(), "int");
    ASSERT_EQ(functions[1]->name(), "dFoo");
    ASSERT_EQ(functions[1]->retType(), "double");
}

TEST(Parser, varTest) {
    const char *input = R"META(
        package test;
        int foo(int x)
        {
            int y = 2*x;
            int z;
            z = x*x;
            return z + y - 3;
        }
    )META";
    meta::Parser parser;
    std::shared_ptr<meta::Package> root(nullptr);
    ASSERT_NO_THROW(root = std::dynamic_pointer_cast<meta::Package>(parser.parse(input)));
    auto varDeclarations = root->getChildren<meta::VarDecl>(2);
    ASSERT_EQ(varDeclarations.size(), 2);
    ASSERT_EQ(varDeclarations[0]->type(), "int");
    ASSERT_EQ(varDeclarations[0]->name(), "y");
    ASSERT_TRUE(varDeclarations[0]->inited());

    ASSERT_EQ(varDeclarations[1]->type(), "int");
    ASSERT_EQ(varDeclarations[1]->name(), "z");
    ASSERT_FALSE(varDeclarations[1]->inited());

    auto assigments = root->getChildren<meta::Assigment>(-1);
    ASSERT_EQ(assigments.size(), 1);
    ASSERT_EQ(assigments[0]->varName(), "z");
}
