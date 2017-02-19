#pragma once

#include <gtest/gtest.h>

#include "utils/testtools.h"

#include "parser/metaparser.h"
#include "parser/var.h"
#include "parser/vardecl.h"

#include "analysers/actions.h"
#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace meta::analysers::tests::resolve_vars {
namespace {

const auto null_pkg = utils::SourceFile::fake(R"META(
    package null;

    extern string question();

    export int half(int x) {return x/2;}

    public int answer() {return 42;}

    protected int tripple(int x) {return 3*x;}

    private int twice(int x) {return 2*x;}
)META", "null.meta");

class SuccessfullResolveFromNull: public ::testing::TestWithParam<utils::SourceFile> {};

TEST_P(SuccessfullResolveFromNull, callNullPkgFunc) {
    const utils::SourceFile input = GetParam();

    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, null_pkg);

    auto ast = parser.ast();
    ASSERT_ANALYSE(resolve(ast, act.dictionary()));

    auto calls = ast->getChildren<Call>(infinitDepth);
    ASSERT_EQ(calls.size(), 1u);
    ASSERT_NE(calls[0]->function(), nullptr);
    EXPECT_EQ(calls[0]->function()->package(), "null");
}

utils::SourceFile successTestData[] = {
    R"META(
        package test;

        int foo(int x) {return x + answer();}
    )META"_fake_src,
    R"META(
        package test;

        int foo(int x) {return half(x);}
    )META"_fake_src,
    R"META(
        package test;

        string foo() {return question();}
    )META"_fake_src
};

INSTANTIATE_TEST_CASE_P(TransparentImportFromNull, SuccessfullResolveFromNull, ::testing::ValuesIn(successTestData));

} // anonymous namespace
} // namespace meta::analysers
