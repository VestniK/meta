
#include <gtest/gtest.h>

#include "utils/testtools.h"

#include "parser/metaparser.h"
#include "parser/var.h"
#include "parser/vardecl.h"

#include "analysers/actions.h"
#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace meta::analysers {
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
    ASSERT_ANALYSE(v2::resolve(ast, act.dictionary()));
    auto vars = ast->getChildren<Var>(infinitDepth);
    ASSERT_EQ(vars.size(), 1u);
    ASSERT_NE(vars[0]->declaration(), nullptr);
    EXPECT_TRUE(vars[0]->declaration()->flags() & VarFlags::argument);
    EXPECT_EQ(vars[0]->declaration()->name(), "x");
}

} // anonymous namespace
} // namespace meta::analysers
