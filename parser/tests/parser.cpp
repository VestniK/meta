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
        functions.push_back(node->name());
    }

    std::string package;
    std::vector<std::string> functions;
};

TEST(Parser, zeroParamFunc) {
    const char *input = "package test; foo() {return 5;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 1);
    ASSERT_EQ(actions.functions[0], "foo");
}

TEST(Parser, oneParamFunc) {
    const char *input = "package test; foo(x) {return 5*x*x - 2*x + 3;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 1);
    ASSERT_EQ(actions.functions[0], "foo");
}

TEST(Parser, twoParamFunc) {
    const char *input = "package test; foo(x, y) {return 5*x + 6/y;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 1);
    ASSERT_EQ(actions.functions[0], "foo");
}

TEST(Parser, twoFunc) {
    const char *input = "package test; foo(x) {return 5*x;}\nbar(x) {return x/5;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 2);
    ASSERT_EQ(actions.functions[0], "foo");
    ASSERT_EQ(actions.functions[1], "bar");
}

TEST(Parser, funcCall) {
    const char *input = "package test; foo(x) {return 5*x;}\nbar(y) {return 5*foo(y/5);}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    parser.setNodeActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, "test");
    ASSERT_EQ(actions.functions.size(), 2);
    ASSERT_EQ(actions.functions[0], "foo");
    ASSERT_EQ(actions.functions[1], "bar");
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
