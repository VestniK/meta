#include <sstream>

#include <gtest/gtest.h>

#include "generators/cppgen/cppwriter.h"

using namespace meta::generators::cppgen;
using namespace std::string_literals;

TEST(CppWriterTests, nestedNamespace) {
    std::ostringstream out;
    {
        CppWriter writer(out);
        writer.setPackage("a.b.c.d");
        writer.setPackage("a.b.e.f");
        writer.setPackage("a.g.h");
        writer.setPackage("p.q.l");
        writer.setPackage("f");
    }
    const auto expected =
R"(namespace a {
namespace b {
namespace c {
namespace d {
}
}
namespace e {
namespace f {
}
}
}
namespace g {
namespace h {
}
}
}
namespace p {
namespace q {
namespace l {
}
}
}
namespace f {
}
)"s
    ;
    ASSERT_EQ(expected, out.str());
}
