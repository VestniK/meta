#include <gtest/gtest.h>

#include "parser/metaparser.h"

TEST(Parser, zeroParamFunc) {
    const char *input = "foo() {return 5;}";
    meta::Parser parser;
    ASSERT_NO_THROW(parser.parse(input));
}

TEST(Parser, oneParamFunc) {
    const char *input = "foo(x) {return 5*x*x - 2*x + 3;}";
    meta::Parser parser;
    ASSERT_NO_THROW(parser.parse(input));
}

TEST(Parser, twoParamFunc) {
    const char *input = "foo(x, y) {return 5*x + 6/y;}";
    meta::Parser parser;
    ASSERT_NO_THROW(parser.parse(input));
}

TEST(Parser, twoFunc) {
    const char *input = "foo(x) {return 5*x;}\nbar(x) {return x/5;}";
    meta::Parser parser;
    ASSERT_NO_THROW(parser.parse(input));
}

TEST(Parser, funcCall) {
    const char *input = "foo(x) {return 5*x;}\nbar(y) {return 5*foo(y/5);}";
    meta::Parser parser;
    ASSERT_NO_THROW(parser.parse(input));
}
