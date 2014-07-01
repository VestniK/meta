#include <cassert>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "parser/metaparser.h"
#include "parser/metanodes.h"

class TestActions: public meta::ParseActions, public meta::NodeActions
{
public:
    virtual void onPackage(const meta::StackFrame *start, size_t size) override
    {
        assert(size == 3);
        package.assign(start[1].start, start[1].end - start[1].start);
    }

    virtual void onFunction(std::shared_ptr<meta::Function> node) override
    {
        functions.push_back(node);
    }

    std::string package;
    std::vector< std::shared_ptr<meta::Function> > functions;
};

TEST(Parser, zeroParamFunc) {
    const char *input = "package test; int foo() {return 5;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 1);
    ASSERT_EQ(actions.functions[0]->name(), "foo");
    ASSERT_EQ(actions.functions[0]->args().size(), 0);
}

TEST(Parser, oneParamFunc) {
    const char *input = "package test; int foo(int x) {return 5*x*x - 2*x + 3;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 1);
    ASSERT_EQ(actions.functions[0]->name(), "foo");
    const auto args = actions.functions[0]->args();
    ASSERT_EQ(args.size(), 1);
    ASSERT_EQ(args[0]->type(), "int");
    ASSERT_EQ(args[0]->name(), "x");
}

TEST(Parser, twoParamFunc) {
    const char *input = "package test; int foo(int x, int y) {return 5*x + 6/y;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 1);
    ASSERT_EQ(actions.functions[0]->name(), "foo");
    const auto args = actions.functions[0]->args();
    ASSERT_EQ(args.size(), 2);
    ASSERT_EQ(args[0]->type(), "int");
    ASSERT_EQ(args[0]->name(), "x");
    ASSERT_EQ(args[1]->type(), "int");
    ASSERT_EQ(args[1]->name(), "y");
}

TEST(Parser, twoFunc) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int x) {return x/5;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 2);

    ASSERT_EQ(actions.functions[0]->name(), "foo");
    const auto args1 = actions.functions[0]->args();
    ASSERT_EQ(args1.size(), 1);
    ASSERT_EQ(args1[0]->type(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(actions.functions[1]->name(), "bar");
    const auto args2 = actions.functions[1]->args();
    ASSERT_EQ(args2.size(), 1);
    ASSERT_EQ(args2[0]->type(), "int");
    ASSERT_EQ(args2[0]->name(), "x");
}

TEST(Parser, funcCall) {
    const char *input = "package test; int foo(int x) {return 5*x;}\nint bar(int y) {return 5*foo(y/5);}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 2);

    ASSERT_EQ(actions.functions[0]->name(), "foo");
    const auto args1 = actions.functions[0]->args();
    ASSERT_EQ(args1.size(), 1);
    ASSERT_EQ(args1[0]->type(), "int");
    ASSERT_EQ(args1[0]->name(), "x");

    ASSERT_EQ(actions.functions[1]->name(), "bar");
    const auto args2 = actions.functions[1]->args();
    ASSERT_EQ(args2.size(), 1);
    ASSERT_EQ(args2[0]->type(), "int");
    ASSERT_EQ(args2[0]->name(), "y");
}

TEST(Parser, emptyPackage) {
    const char *input = "package test.test;";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test.test");
    ASSERT_EQ(actions.functions.size(), 0);
}

TEST(Parser, funcRetType) {
    const char *input = "package test; int iFoo() {return 0;}\ndouble dFoo() {return 0;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.functions.size(), 2);
    ASSERT_EQ(actions.functions[0]->name(), "iFoo");
    ASSERT_EQ(actions.functions[0]->retType(), "int");
    ASSERT_EQ(actions.functions[1]->name(), "dFoo");
    ASSERT_EQ(actions.functions[1]->retType(), "double");
}

