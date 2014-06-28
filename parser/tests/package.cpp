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
    const char *content = "package unit.test; foo() {return 5;}";
    Package pkg;
    pkg.parse("test.meta", content);
    ASSERT_EQ(pkg.name, "unit.test");
    ASSERT_EQ(pkg.functions.size(), 1);
    ASSERT_EQ(pkg.functions.at("foo")->name(), "foo");
}

TEST(Package, twoFunctions) {
    const char *content = "package unit.test; sqr(int x) {return x*x;} bar(int x1, int x2) {return x2 - x1;}";
    Package pkg;
    pkg.parse("test.meta", content);
    ASSERT_EQ(pkg.name, "unit.test");
    ASSERT_EQ(pkg.functions.size(), 2);
    ASSERT_EQ(pkg.functions.at("sqr")->name(), "sqr");
    ASSERT_EQ(pkg.functions.at("bar")->name(), "bar");
}

TEST(Package, nameConflict) {
    const char *content = "package unit.test; foo(int x) {return x*x;} foo(int x1, int x2) {return x2 - x1;}";
    Package pkg;
    try {
    ASSERT_THROW(pkg.parse("test.meta", content), std::runtime_error);
    } catch(std::runtime_error &e) {
        printf("caught an std::runtime_error %s\n", e.what());
    } catch(const std::exception &e) {
        printf("caught an std::exception %s\n", e.what());
    } catch(...)  {
        printf("caught something strange\n");
    }
}