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

TEST(ResolveVars, simple) {
    const utils::SourceFile input = R"META(
        package test;

        extern int dist(int y);

        int foo(int x) {
            return dist(x+5);
        }
    )META"_fake_src;

    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();
    ASSERT_ANALYSE(resolve(ast, act.dictionary()));
    auto vars = ast->getChildren<Var>(infinitDepth);
    ASSERT_EQ(vars.size(), 1u);
    ASSERT_NE(vars[0]->declaration(), nullptr);
    EXPECT_TRUE(vars[0]->declaration()->flags() & VarFlags::argument);
    EXPECT_EQ(vars[0]->declaration()->name(), "x");
}

TEST(ResolveVars, complexUsage) {
    const utils::SourceFile input = R"META(
        package test;

        int abs(int x) {
            auto res = x;
            if (x < 0)
                res = -x;
            return res;
        }
    )META"_fake_src;

    Parser parser;
    Actions act;
    parser.setNodeActions(&act);
    parser.setParseActions(&act);
    ASSERT_PARSE(parser, input);
    auto ast = parser.ast();
    ASSERT_ANALYSE(resolve(ast, act.dictionary()));
    auto vars = ast->getChildren<Var>(infinitDepth);
    ASSERT_EQ(vars.size(), 5u);
    for (auto* var: vars) {
        ASSERT_NE(var->declaration(), nullptr);
        if (var->name() == "x") {
            EXPECT_TRUE(var->declaration()->flags() & VarFlags::argument);
            EXPECT_EQ(var->declaration()->name(), "x");
        } else if (var->name() == "res") {
            EXPECT_FALSE(var->declaration()->flags() & VarFlags::argument);
            EXPECT_EQ(var->declaration()->name(), "res");
        } else
            ADD_FAILURE() <<
                var->source().path().string() << var->tokens().linenum() << ':' << var->tokens().colnum() <<
                ": unexpected var name '" << var->name() << '\'';
    }
}

} // anonymous namespace
} // namespace meta::analysers
