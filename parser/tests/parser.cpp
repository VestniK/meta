#include <cassert>
#include <string>

#include <gtest/gtest.h>

#include "parser/metaparser.h"

class TestActions: public meta::ParseActions
{
public:
    virtual void onPackage(const meta::StackFrame *start, size_t size)
    {
        assert(size == 3);
        package.assign(start[1].start, start[1].end - start[1].start);
    }

    std::string package;
};

TEST(Parser, zeroParamFunc) {
    const char *input = "package test; foo() {return 5;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, std::string("test"));
}

TEST(Parser, oneParamFunc) {
    const char *input = "package test; foo(x) {return 5*x*x - 2*x + 3;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, std::string("test"));
}

TEST(Parser, twoParamFunc) {
    const char *input = "package test; foo(x, y) {return 5*x + 6/y;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, std::string("test"));
}

TEST(Parser, twoFunc) {
    const char *input = "package test; foo(x) {return 5*x;}\nbar(x) {return x/5;}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, std::string("test"));
}

TEST(Parser, funcCall) {
    const char *input = "package test; foo(x) {return 5*x;}\nbar(y) {return 5*foo(y/5);}";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, std::string("test"));
}

TEST(Parser, emptyPackage) {
    const char *input = "package test.test;";
    meta::Parser parser;
    TestActions actions;
    parser.setParseActions(&actions);
    ASSERT_NO_THROW(parser.parse(input));
    ASSERT_EQ(actions.package, std::string("test.test"));
}
