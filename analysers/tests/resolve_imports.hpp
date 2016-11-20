#include <gtest/gtest.h>

#include "utils/testtools.h"
#include "utils/sourcefile.h"

#include "typesystem/typesstore.h"

#include "parser/function.h"
#include "parser/import.h"
#include "parser/metaparser.h"
#include "parser/struct.h"

#include "analysers/actions.h"
#include "analysers/declconflicts.h"
#include "analysers/semanticerror.h"
#include "analysers/resolver.h"

namespace meta::analysers::tests::resolve_imports {
namespace {

const auto lib = utils::SourceFile::fake(R"META(
    package test.lib;

    extern
    struct Point {
        int x;
        int y;
    }

    export
    struct Point3d {
        int x;
        int y;
        int z;
    }

    public
    struct PointF {
        double x;
        double y;
    }

    protected
    struct PointF3d {
        double x;
        double y;
        double z;
    }

    private
    struct Color {
        int red;
        int green;
        int blue;
    }

    public int foo(int x) {return x;}
    private int privFoo(int x) {return x;}
    protected int protFoo(int x) {return x;}

    protected void overload2() {return;}
    private int overload2(int x) {return x;}

    extern int overloadAll(string x);
    export string overloadAll(string x) {return x;}
    public bool overloadAll(bool x) {return x;}
    protected void overloadAll() {return;}
    private int overloadAll(int x) {return x;}

    public int visibleOverloads(int x) {return x;}
    public bool visibleOverloads(bool x) {return x;}

    public string bar(string x) {return x;}
)META", "lib.meta");

TEST(ResolveImports, properStructImport) {
    const utils::SourceFile input = R"META(
        package test.lib.subpkg;

        import test.lib.Point;
        import test.lib.Point3d;
        import test.lib.PointF;
        import test.lib.PointF3d;
    )META"_fake_src;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    resolve(ast, act.dictionary(), typestore);
    auto imports = ast->getChildren<Import>(infinitDepth);
    ASSERT_EQ(imports.size(), 4u);
    for (auto import: imports) {
        ASSERT_EQ(import->importedDeclarations().size(), 1u);
        auto target = dynamic_cast<Struct*>(import->importedDeclarations()[0]);
        ASSERT_NE(target, nullptr);
        EXPECT_EQ(target->name(), import->target());
        EXPECT_EQ(target->package(), import->targetPackage());
    }
}

TEST(ResolveImports, simpleFuncImport) {
    const utils::SourceFile input = R"META(
        package test.lib.subpkg;

        import test.lib.foo;
        import test.lib.protFoo;
    )META"_fake_src;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    resolve(ast, act.dictionary(), typestore);
    auto imports = ast->getChildren<Import>();
    ASSERT_EQ(imports.size(), 2u);
    for (auto import: imports) {
        ASSERT_EQ(import->importedDeclarations().size(), 1u);
        auto func = dynamic_cast<Function*>(import->importedDeclarations()[0]);
        ASSERT_NE(func, nullptr);
        EXPECT_EQ(func->name(), import->target());
        EXPECT_EQ(func->package(), import->targetPackage());
    }
}

TEST(ResolveImports, partialFuncImport) {
    const utils::SourceFile input = R"META(
        package test.lib.subpkg;

        import test.lib.overload2;
        import test.lib.overloadAll;
    )META"_fake_src;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    resolve(ast, act.dictionary(), typestore);
    auto imports = ast->getChildren<Import>();
    ASSERT_EQ(imports.size(), 2u);

    // overload2
    ASSERT_EQ(imports[0]->importedDeclarations().size(), 1u);
    auto func = dynamic_cast<Function*>(imports[0]->importedDeclarations()[0]);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->visibility(), Visibility::Protected);
    EXPECT_EQ(func->name(), imports[0]->target());
    EXPECT_EQ(func->package(), imports[0]->targetPackage());

    // overloadAll
    ASSERT_EQ(imports[1]->importedDeclarations().size(), 4u);
    std::vector<Function*> funcs;
    for (auto decl: imports[1]->importedDeclarations()) {
        funcs.push_back(dynamic_cast<Function*>(decl));
        ASSERT_NE(funcs.back(), nullptr);
    }
    std::sort(funcs.begin(), funcs.end(), [](Function* lhs, Function* rhs) {
        return lhs->visibility() < rhs->visibility();
    });
    funcs.erase(std::unique(funcs.begin(), funcs.end(), [](Function* lhs, Function* rhs) {
        return lhs->visibility() == rhs->visibility();
    }), funcs.end());
    ASSERT_EQ(funcs.size(), 4u);
    for (auto func: funcs) {
        EXPECT_NE(func->visibility(), Visibility::Private);
        EXPECT_EQ(func->name(), imports[1]->target());
        EXPECT_EQ(func->package(), imports[1]->targetPackage());
    }
}

TEST(ResolveImports, allFuncImport) {
    const utils::SourceFile input = R"META(
        package test.lib.subpkg;

        import test.lib.visibleOverloads;
    )META"_fake_src;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    resolve(ast, act.dictionary(), typestore);
    auto imports = ast->getChildren<Import>();
    ASSERT_EQ(imports.size(), 1u);

    ASSERT_EQ(imports[0]->importedDeclarations().size(), 2u);
    EXPECT_NE(imports[0]->importedDeclarations()[0], imports[0]->importedDeclarations()[1]);
    for (auto decl: imports[0]->importedDeclarations()) {
        auto func = dynamic_cast<Function*>(decl);
        ASSERT_NE(func, nullptr);
        EXPECT_EQ(func->name(), imports[0]->target());
        EXPECT_EQ(func->package(), imports[0]->targetPackage());
    }
}

TEST(ResolveImports, filterOutProtectedFuncImport) {
    const utils::SourceFile input = R"META(
        package test;

        import test.lib.overloadAll;
    )META"_fake_src;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    resolve(ast, act.dictionary(), typestore);
    auto imports = ast->getChildren<Import>();
    ASSERT_EQ(imports.size(), 1u);

    ASSERT_EQ(imports[0]->importedDeclarations().size(), 3u);
    EXPECT_NE(imports[0]->importedDeclarations()[0], imports[0]->importedDeclarations()[1]);
    EXPECT_NE(imports[0]->importedDeclarations()[1], imports[0]->importedDeclarations()[2]);
    EXPECT_NE(imports[0]->importedDeclarations()[0], imports[0]->importedDeclarations()[2]);
    for (auto decl: imports[0]->importedDeclarations()) {
        auto func = dynamic_cast<Function*>(decl);
        ASSERT_NE(func, nullptr);
        EXPECT_NE(func->visibility(), Visibility::Private);
        EXPECT_NE(func->visibility(), Visibility::Protected);
        EXPECT_EQ(func->name(), imports[0]->target());
        EXPECT_EQ(func->package(), imports[0]->targetPackage());
    }
}

TEST(ResolveImports, importAsOverload) {
    const utils::SourceFile input = R"META(
        package test;

        import test.lib.foo;
        import test.lib.bar as foo;
    )META"_fake_src;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    resolve(ast, act.dictionary(), typestore);
    auto imports = ast->getChildren<Import>();
    ASSERT_EQ(imports.size(), 2u);
    EXPECT_EQ(imports[0]->name(), imports[1]->name());

    EXPECT_EQ(imports[0]->target(), "foo");
    EXPECT_EQ(imports[0]->targetPackage(), "test.lib");

    EXPECT_EQ(imports[1]->target(), "bar");
    EXPECT_EQ(imports[1]->targetPackage(), "test.lib");
}

TEST(ResolveImports, importAsRenamedOverload) {
    const utils::SourceFile input = R"META(
        package test;

        import test.lib.foo as baz;
        import test.lib.bar as baz;
    )META"_fake_src;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    resolve(ast, act.dictionary(), typestore);
    auto imports = ast->getChildren<Import>();
    ASSERT_EQ(imports.size(), 2u);
    EXPECT_EQ(imports[0]->name(), imports[1]->name());

    EXPECT_EQ(imports[0]->target(), "foo");
    EXPECT_EQ(imports[0]->targetPackage(), "test.lib");

    EXPECT_EQ(imports[1]->target(), "bar");
    EXPECT_EQ(imports[1]->targetPackage(), "test.lib");
}

TEST(ResolveImports, importExtendingOverload) {
    const utils::SourceFile input = R"META(
        package test;

        import test.lib.foo;

        int foo(int x, int y) {return x + y;}
    )META"_fake_src;
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    ASSERT_ANALYSE(resolve(ast, act.dictionary(), typestore));
    auto imports = ast->getChildren<Import>();
    ASSERT_EQ(imports.size(), 1u);
    EXPECT_EQ(imports[0]->name(), "foo");
}

struct ImportErrors: public utils::ErrorTest {};

utils::ErrorTestData testData[] = {
    // Import from current pkg
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.A;

            struct A {int x;}
        )META"_fake_src,
        .errMsg = "Import of a declaration from the current package is meaningless"
    },
    // invalid import targets
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import no.such.pkg.Color;
        )META"_fake_src,
        .errMsg = "No such package 'no.such.pkg'"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.NoSuchDecl;
        )META"_fake_src,
        .errMsg = "Declaration 'NoSuchDecl' not found in package 'test.lib'"
    },
    // structs import violating visibility restrictions
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Color;
        )META"_fake_src,
        .errMsg = "Struct 'Color' is private in the package 'test.lib'"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.PointF3d;
        )META"_fake_src,
        .errMsg = "Struct 'PointF3d' is protected in the package 'test.lib' which is not parent of current package 'test'"
    },
    // struct import with conflicts
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Point;

            struct Point {int x; int y;}
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point' conflicts with other declarations.
test.meta:6:13: notice: Struct 'test.Point')"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Point as Point2d;

            struct Point2d {int x; int y;}
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point2d' conflicts with other declarations.
test.meta:6:13: notice: Struct 'test.Point2d')"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Point as point;

            void point() {return;}
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point' as 'point' conflicts with other declarations.
test.meta:6:13: notice: Function 'test.point()')"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Point as point;

            void point() {return;}
            void point(int x) {return;}
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point' as 'point' conflicts with other declarations.
test.meta:6:13: notice: Function 'test.point()'
test.meta:7:13: notice: Function 'test.point(int)')"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Point;

            void Point() {return;}
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point' conflicts with other declarations.
test.meta:6:13: notice: Function 'test.Point()')"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Point;

            void Point() {return;}
            void Point(int x) {return;}
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point' conflicts with other declarations.
test.meta:6:13: notice: Function 'test.Point()'
test.meta:7:13: notice: Function 'test.Point(int)')"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Point;
            import test.lib.Point3d as Point;
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point3d' as 'Point' conflicts with other declarations.
lib.meta:4:5: notice: Struct 'test.lib.Point'
	imported as 'Point' here: test.meta:4:13)"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.Point3d as Point;
            import test.lib.Point;
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point' conflicts with other declarations.
lib.meta:10:5: notice: Struct 'test.lib.Point3d'
	imported as 'Point' here: test.meta:4:13)"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.foo as bar;
            import test.lib.Point3d as bar;
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.Point3d' as 'bar' conflicts with other declarations.
lib.meta:37:5: notice: Function 'test.lib.foo(int)'
	imported as 'bar' here: test.meta:4:13)"
    },
    // func imports with conflicts
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.foo as Point;

            struct Point {int x; int y;}
        )META"_fake_src,
        .errMsg =
R"(Import of 'test.lib.foo' as 'Point' conflicts with other declarations.
test.meta:6:13: notice: Struct 'test.Point')"
    },
    // func imports violating visibility rules
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.privFoo;
        )META"_fake_src,
        .errMsg =
R"(Function 'privFoo' from the package 'test.lib' has no overloads visible from the current package 'test'
lib.meta:38:5: notice: Function 'test.lib.privFoo(int)' is private)"
    },
    utils::ErrorTestData{
        .input = R"META(
            package test;

            import test.lib.protFoo;
        )META"_fake_src,
        .errMsg =
R"(Function 'protFoo' from the package 'test.lib' has no overloads visible from the current package 'test'
lib.meta:39:5: notice: Function 'test.lib.protFoo(int)' is protected)"
    },
    {
        .input = R"META(
            package test;

            import test.lib.overload2;
        )META"_fake_src,
        .errMsg =
            "Function 'overload2' from the package 'test.lib' has no overloads visible from the current package 'test'\n"
            "lib.meta:41:5: notice: Function 'test.lib.overload2()' is protected\n"
            "lib.meta:42:5: notice: Function 'test.lib.overload2(int)' is private"
    }
};
INSTANTIATE_TEST_CASE_P(ResolveImports, ImportErrors, ::testing::ValuesIn(testData));

TEST_P(ImportErrors, importErrors) {
    auto param = GetParam();
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, param.input);
    ASSERT_PARSE(parser, lib);
    auto ast = parser.ast();
    typesystem::TypesStore typestore;
    try {
        resolve(ast, act.dictionary(), typestore);
        FAIL() << "Error was not detected: " << param.errMsg;
    } catch (const SemanticError& err) {
        EXPECT_EQ(param.errMsg, err.what()) << err.what();
    }
}

} // anonymous namespace
} // namespace meta::analysers
