#include <gtest/gtest.h>

#include "utils/testtools.h"

#include "typesystem/typesstore.h"

#include "parser/metaparser.h"
#include "parser/function.h"
#include "parser/call.h"

#include "analysers/actions.h"
#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace meta::analysers::tests::resolve_call {
namespace {

TEST(ResolveCall, simple) {
    const auto input = R"META(
        package test;

        int foo(int x, int y) {return x + y - 42;}

        int bar(bool b) {
            if (b)
                return foo(123, 456);
            return foo(42, 15);
        }
    )META"_fake_src;

    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    ASSERT_ANALYSE(resolve(ast, act.dictionary(), typestore));
    auto calls = ast->getChildren<Call>(infinitDepth);
    ASSERT_EQ(calls.size(), 2u);
    ASSERT_NE(calls[0]->function(), nullptr);
    ASSERT_NE(calls[1]->function(), nullptr);
    EXPECT_EQ(calls[0]->function(), calls[1]->function());
    EXPECT_EQ(calls[0]->function()->name(), "foo");
}

TEST(ResolveCall, recursive) {
    const auto input = R"META(
        package test;

        int fact(int n) {
            if (n == 0)
                return 1;
            return n*fact(n - 1);
        }
    )META"_fake_src;

    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    ASSERT_ANALYSE(resolve(ast, act.dictionary(), typestore));
    auto calls = ast->getChildren<Call>(infinitDepth);
    ASSERT_EQ(calls.size(), 1u);
    ASSERT_NE(calls[0]->function(), nullptr);
    EXPECT_EQ(calls[0]->function()->name(), "fact");
}

TEST(ResolveCall, inderectRecursive) {
    const auto input = R"META(
        package test;

        int foo(int n) {
            if (n < 0)
                return bar(-n);
            return 2*n;
        }

        int bar(int n) {
            if (n > 100)
                return foo(n/2);
            return foo(2*n);
        }
    )META"_fake_src;

    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    ASSERT_ANALYSE(resolve(ast, act.dictionary(), typestore));
    auto calls = ast->getChildren<Call>(infinitDepth);
    ASSERT_EQ(calls.size(), 3u);
    ASSERT_NE(calls[0]->function(), nullptr);
    ASSERT_NE(calls[1]->function(), nullptr);
    ASSERT_NE(calls[2]->function(), nullptr);

    EXPECT_EQ(calls[0]->function()->name(), "bar");
    EXPECT_EQ(calls[1]->function()->name(), "foo");
    EXPECT_EQ(calls[2]->function()->name(), "foo");
    EXPECT_EQ(calls[1]->function(), calls[2]->function());
}

TEST(ResolveCall, imported) {
    const auto input1 = utils::SourceFile::fake(R"META(
        package test.a;

        import test.b.baz as bar;

        public int foo(int n) {
            if (n < 0)
                return bar(-n);
            return 2*n;
        }
    )META", "test/a.meta");
    const auto input2 = utils::SourceFile::fake(R"META(
        package test.b;

        import test.a.foo;

        public int baz(int n) {
            if (n > 100)
                return foo(n/2);
            return foo(2*n);
        }
    )META", "test/b.meta");

    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, input1);
    ASSERT_PARSE(parser, input2);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    ASSERT_ANALYSE(resolve(ast, act.dictionary(), typestore));
    auto calls = ast->getChildren<Call>(infinitDepth);
    ASSERT_EQ(calls.size(), 3u);
    ASSERT_NE(calls[0]->function(), nullptr);
    ASSERT_NE(calls[1]->function(), nullptr);
    ASSERT_NE(calls[2]->function(), nullptr);

    EXPECT_EQ(calls[0]->function()->name(), "baz");
    EXPECT_EQ(calls[0]->functionName(), "bar");
    EXPECT_EQ(calls[1]->function()->name(), "foo");
    EXPECT_EQ(calls[2]->function()->name(), "foo");
    EXPECT_EQ(calls[1]->function(), calls[2]->function());
}

TEST(ResolveCall, samePkgAnotherFile) {
    const auto input1 = utils::SourceFile::fake(R"META(
        package test;

        int foo(int n) {
            if (n < 0)
                return bar(-n);
            return 2*n;
        }
    )META", "test1.meta");
    const auto input2 = utils::SourceFile::fake(R"META(
        package test;

        public int bar(int n) {
            if (n > 100)
                return foo(n/2);
            return foo(2*n);
        }
    )META", "test2.meta");

    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, input1);
    ASSERT_PARSE(parser, input2);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    ASSERT_ANALYSE(resolve(ast, act.dictionary(), typestore));
    auto calls = ast->getChildren<Call>(infinitDepth);
    ASSERT_EQ(calls.size(), 3u);
    ASSERT_NE(calls[0]->function(), nullptr);
    ASSERT_NE(calls[1]->function(), nullptr);
    ASSERT_NE(calls[2]->function(), nullptr);

    EXPECT_EQ(calls[0]->function()->name(), "bar");
    EXPECT_EQ(calls[1]->function()->name(), "foo");
    EXPECT_EQ(calls[2]->function()->name(), "foo");
    EXPECT_EQ(calls[1]->function(), calls[2]->function());
}

class ResolveCallErrors: public utils::ErrorTest {};

TEST_P(ResolveCallErrors, resolveErrors) {
    const auto& param = GetParam();
    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, param.input);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    try {
        resolve(ast, act.dictionary(), typestore);
        FAIL() << "Error was not detected: " << param.errMsg;
    } catch (const SemanticError& err) {
        EXPECT_EQ(err.what(), param.errMsg) << err.what();
    }
}

INSTANTIATE_TEST_CASE_P(Resolver, ResolveCallErrors, ::testing::Values(
    utils::ErrorTestData{
        .input=R"META(
            package test;

            extern int bar(int x, int y);

            int foo(int x) {
                return bar(x, 42, 65);
            }
        )META"_fake_src,
        .errMsg=R"(Function 'test.bar(int, int)' is called with incorrect number of arguments)"
    },
    utils::ErrorTestData{
        .input=R"META(
            package test;

            int foo(int x) {
                return bar(x, 42, 65);
            }
        )META"_fake_src,
        .errMsg=R"(Unresolved function call 'bar')"
    }
));

} // anonymous namespace
} // namespace meta::analysers
