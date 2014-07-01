#include <stdexcept>

#include <gtest/gtest.h>

#include "parser/package.h"

TEST(Package, noFunctions) {
    const char *content = "package unit.test;";
    Package pkg;
    pkg.parse("test.meta", content);
    ASSERT_EQ(pkg.name, "unit.test");
    ASSERT_EQ(pkg.functions.size(), 0);
}

TEST(Package, oneFunction) {
    const char *content = "package unit.test; int foo() {return 5;}";
    Package pkg;
    pkg.parse("test.meta", content);
    ASSERT_EQ(pkg.name, "unit.test");
    ASSERT_EQ(pkg.functions.size(), 1);
    ASSERT_EQ(pkg.functions.at("foo")->name(), "foo");
}

TEST(Package, twoFunctions) {
    const char *content = "package unit.test; int sqr(int x) {return x*x;} int bar(int x1, int x2) {return x2 - x1;}";
    Package pkg;
    pkg.parse("test.meta", content);
    ASSERT_EQ(pkg.name, "unit.test");
    ASSERT_EQ(pkg.functions.size(), 2);
    ASSERT_EQ(pkg.functions.at("sqr")->name(), "sqr");
    ASSERT_EQ(pkg.functions.at("bar")->name(), "bar");
}

TEST(Package, nameConflict) {
    const char *content = "package unit.test; int foo(int x) {return x*x;} int foo(int x1, int x2) {return x2 - x1;}";
    Package pkg;
    ASSERT_THROW(pkg.parse("test.meta", content), std::runtime_error);
}
