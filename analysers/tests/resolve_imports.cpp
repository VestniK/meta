#include <gtest/gtest.h>

#include "utils/testtools.h"

#include "parser/function.h"
#include "parser/import.h"
#include "parser/metaparser.h"
#include "parser/struct.h"

#include "analysers/actions.h"
#include "analysers/semanticerror.h"
#include "analysers/resolver.h"

namespace meta::analysers {

const utils::string_view lib = R"META(
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

    int foo(int x) {return x;}
)META";

TEST(ResolveImports, properStructImport) {
    utils::string_view input = R"META(
        package test.lib.subpkg;

        import test.lib.Point;
        import test.lib.Point3d;
        import test.lib.PointF;
        import test.lib.PointF3d;
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
    ASSERT_EQ(imports.size(), 4u);
    utils::string_view names[] = {"Point", "Point3d", "PointF", "PointF3d"};
    auto nameIt = std::begin(names);
    for (auto import: imports) {
        ASSERT_EQ(import->importedDeclarations().size(), 1u);
        auto target = dynamic_cast<Struct*>(import->importedDeclarations()[0]);
        ASSERT_NE(target, nullptr);
        EXPECT_EQ(target->name(), *(nameIt++));
        EXPECT_EQ(target->package(), "test.lib");
    }
}

namespace {

struct TestData {
    utils::string_view input;
    utils::string_view errMsg;
};


std::ostream& operator<< (std::ostream& out, const TestData& dat) {
    out << "=== input ===\n" << dat.input << "\n=== expected error ===\n" << dat.errMsg;
    return out;
}

struct ResolveImports: public ::testing::TestWithParam<TestData> {};

INSTANTIATE_TEST_CASE_P(ImportErrors, ResolveImports, ::testing::Values(
    // Import from current pkg
    TestData{
        .input = R"META(
            package test;

            import test.A;

            struct A {int x;}
        )META",
        .errMsg = "Import of a declaration from the current package is meaningless"
    },
    // invalid import targets
    TestData{
        .input = R"META(
            package test;

            import no.such.pkg.Color;
        )META",
        .errMsg = "No such package 'no.such.pkg'"
    },
    TestData{
        .input = R"META(
            package test;

            import test.lib.NoSuchDecl;
        )META",
        .errMsg = "Declaration 'NoSuchDecl' not found in package 'test.lib'"
    },
    // structs import violating visibility restrictions
    TestData{
        .input = R"META(
            package test;

            import test.lib.Color;
        )META",
        .errMsg = "Struct 'Color' is private in the package 'test.lib'"
    },
    TestData{
        .input = R"META(
            package test;

            import test.lib.PointF3d;
        )META",
        .errMsg = "Struct 'PointF3d' is protected in the package 'test.lib' which is not parent of current package 'test'"
    },
    // struct import with conflicts
    TestData{
        .input = R"META(
            package test;

            import test.lib.Point;

            struct Point {int x; int y;}
        )META",
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point' conflicts with other declarations.
notice: test.meta:6:13: Struct 'test.Point')"
    },
        TestData{
        .input = R"META(
            package test;

            import test.lib.Point as Point2d;

            struct Point2d {int x; int y;}
        )META",
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point2d' conflicts with other declarations.
notice: test.meta:6:13: Struct 'test.Point2d')"
    },
    TestData{
        .input = R"META(
            package test;

            import test.lib.Point as point;

            void point() {return;}
        )META",
        .errMsg =
R"(Import of 'test.lib.Point' as 'point' conflicts with other declarations.
notice: test.meta:6:13: Function 'test.point()')"
    },
    TestData{
        .input = R"META(
            package test;

            import test.lib.Point as point;

            void point() {return;}
            void point(int x) {return;}
        )META",
        .errMsg =
R"(Import of 'test.lib.Point' as 'point' conflicts with other declarations.
notice: test.meta:6:13: Function 'test.point()'
notice: test.meta:7:13: Function 'test.point(int)')"
    },
    TestData{
        .input = R"META(
            package test;

            import test.lib.Point;

            void Point() {return;}
        )META",
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point' conflicts with other declarations.
notice: test.meta:6:13: Function 'test.Point()')"
    },
    TestData{
        .input = R"META(
            package test;

            import test.lib.Point;

            void Point() {return;}
            void Point(int x) {return;}
        )META",
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point' conflicts with other declarations.
notice: test.meta:6:13: Function 'test.Point()'
notice: test.meta:7:13: Function 'test.Point(int)')"
    },
    TestData{
        .input = R"META(
            package test;

            import test.lib.Point;
            import test.lib.Point3d as Point;
        )META",
        .errMsg =
R"(Import of 'test.lib.Point3d' as 'Point' conflicts with other declarations.
notice: lib.meta:4:5: Struct 'test.lib.Point'
	imported as 'Point' here: test.meta:4:13)"
    },
    TestData{
        .input = R"META(
            package test;

            import test.lib.Point3d as Point;
            import test.lib.Point;
        )META",
        .errMsg =
R"(Import of 'test.lib.Point' as 'Point' conflicts with other declarations.
notice: lib.meta:10:5: Struct 'test.lib.Point3d'
	imported as 'Point' here: test.meta:4:13)"
    },
    // func imports with conflicts
    TestData{
        .input = R"META(
            package test;

            import test.lib.foo as Point;

            struct Point {int x; int y;}
        )META",
        .errMsg =
R"(Import of 'test.lib.foo' as 'Point' conflicts with other declarations.
notice: test.meta:6:13: Struct 'test.Point')"
    }
));

} // anonymous namespace

TEST_P(ResolveImports, importErrors) {
    auto param = GetParam();
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    ASSERT_PARSE(parser, "test.meta", param.input);
    ASSERT_PARSE(parser, "lib.meta", lib);
    auto ast = parser.ast();
    try {
        v2::resolve(ast, act.dictionary());
        FAIL() << "Error was not detected: " << param.errMsg;
    } catch (const SemanticError& err) {
        EXPECT_EQ(err.what(), param.errMsg) << err.what();
    }
}

} // namespace meta::analysers
