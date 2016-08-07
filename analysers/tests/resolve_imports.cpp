#include <gtest/gtest.h>

#include "utils/testtools.h"

#include "parser/function.h"
#include "parser/import.h"
#include "parser/metaparser.h"
#include "parser/struct.h"

#include "analysers/actions.h"
#include "analysers/resolver.h"

namespace meta::analysers {

TEST(ResolveImports, importPublicStruct) {
    utils::string_view lib = R"META(
        package test.lib;

        public struct Point {int x; int y;}
    )META";
    utils::string_view input = R"META(
        package test;

        import test.lib.Point;
    )META";
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, "test.meta", input);
    ASSERT_PARSE(parser, "lib.meta", lib);
    auto ast = parser.ast();
    v2::resolve(ast, act.dictionary());
    auto imports = ast->getChildren<Import>(infinitDepth);
    ASSERT_EQ(imports.size(), 1u);
    ASSERT_EQ(imports[0]->importedDeclarations().size(), 1u);
    auto target = dynamic_cast<Struct*>(imports[0]->importedDeclarations()[0]);
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->name(), "Point");
    EXPECT_EQ(target->package(), "test.lib");
}

} // namespace meta::analysers
